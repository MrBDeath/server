//
// Created by vadim on 6/18/18.
//

#include <sys/socket.h>
#include <unistd.h>
#include "client.h"
#include "log.h"
#include "httpparser/httprequestparser.h"
#include "utils.h"
#include "device.h"
#include "file_reader.h"


CClient::CClient(int fd, const Config &conf) :
    dSocket(fd),
    state(ST_INIT),
    cfg(conf)
{
}



bool CClient::Run()
{
    switch (state)
    {
        case ST_INIT:
        {
            tTimeout = time(nullptr);
            state = ST_PARSE;
            break;
        }
        case ST_PARSE:
        {
            char buf[4096] = {0};
            httpparser::HttpRequestParser http_parser;
            httpparser::HttpRequestParser::ParseResult res;
            timeval tv{0};
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(dSocket, &rfds);
            long int retval = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &rfds, nullptr, nullptr, &tv));
            if (retval < 0)
            {
                ERROR("select() error %s",strerror(errno));
                return true;
            }
            if (FD_ISSET(dSocket, &rfds))
            {
                ssize_t count = TEMP_FAILURE_RETRY(recv(dSocket, buf, sizeof(buf), 0));
                if (count == -1)
                {
                    ERROR("recv() error %s",strerror(errno));
                    return true;
                }

                res = http_parser.parse(http_request, buf, buf + sizeof(buf));
                if (res == httpparser::HttpRequestParser::ParsingError)
                {
                    WARNING("400 Bad Request. Client socket is %d", getFileDesc());
                    return true;
                }
                if(res == httpparser::HttpRequestParser::ParsingCompleted)
                {
                    state = ST_RQ_PARSE;
                }
            }
            //tTimeout = tTimeout - (time(nullptr) - tParse);
            if((time(nullptr) - tTimeout) > cfg.GetTimeout())
            {
                ERROR("recv() error %s",strerror(errno));
                return true;
            }
            break;
        }

        case ST_RQ_PARSE:
        {
            if(http_request.method == "GET")
            {
                std::vector<std::string> command = Utils::get_commands(http_request.uri);
                if(command.size() > 0 && command[0] == "dispenser")
                    m_pExecutor = new CDevice(cfg);
                else
                    m_pExecutor = new CFileReader(cfg);

                m_pExecutor->SetURI(http_request.uri);
                state = ST_EXECUTE_REQ;
            }
            else
            {
                ERROR("501 Not Implemented. Client socket is %s", getFileDesc());
                return true;
            }
            break;
        }
        case ST_EXECUTE_REQ:
        {
            m_pExecutor->Run();
            state = ST_MAKE_RES;
            break;
        }
        case ST_MAKE_RES:
        {
            std::stringstream res;
            res << "HTTP/"
            << http_request.versionMajor << "." << http_request.versionMinor << " "
            << m_pExecutor->GetResType() << " " << m_pExecutor->GetResName() << "\r\n"
            << "Server: C++ server\r\n"
            << "Content-length: " << m_pExecutor->GetResp().length() << "\r\n"
            << "\r\n" << m_pExecutor->GetResp();

            response = res.str();

            tTimeout = cfg.GetTimeout();
            all_sz = 0;
            state = ST_SEND_RES;
            break;
        }
        case ST_SEND_RES:
        {
            if(tTimeout <= 0)
            {
                ERROR("Sending Timeout");
                return true;
            }

            time_t tRes = time(nullptr);
            timeval tv = {0};
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(dSocket,&wfds);
            long int retval = TEMP_FAILURE_RETRY(select(FD_SETSIZE,nullptr,&wfds,nullptr,&tv));
            ssize_t s = 0;
            if(retval < 0)
            {
                ERROR("select() returned with error %d", errno);
                return true;
            }
            if(FD_ISSET(dSocket,&wfds))
            {
                s = TEMP_FAILURE_RETRY(send(dSocket, response.c_str() + all_sz, response.length() - all_sz, 0));
                if (s < 0)
                    return true;
                all_sz += s;
            }
            tTimeout = tTimeout - (time(nullptr) - tRes);
            if(all_sz >= response.length())
                return true;
            break;
        }
    }
    return false;
}


int CClient::getFileDesc()
{
    return dSocket;
}
