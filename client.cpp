//
// Created by vadim on 6/18/18.
//

#include "client.h"

CClient::CClient(int fd, Config &conf) :
    dSocket(fd),
    state(ST_INIT),
    cfg(&conf)
{
}


std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
        tokens.push_back(token);
    return tokens;
}


bool CClient::Run()
{
    switch (state)
    {
        case ST_INIT:
        {
            tTimeout = cfg->GetTimeout();
            state = ST_PARSE;
            break;
        }
        case ST_PARSE:
        {
            if(tTimeout <= 0)
            {
                // timeout
                // return
            }

            time_t tParse = time(nullptr);

            char buf[4098];
            httpparser::HttpRequestParser http_parser;
            httpparser::HttpRequestParser::ParseResult res;
            timeval tv = {0};
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(dSocket, &rfds);
            long int retval = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &rfds, nullptr, nullptr, &tv));
            if (retval < 0)
            {
                //ERROR("Some problems with select()");
                //return -1;
            }
            if (FD_ISSET(dSocket, &rfds))
            {
                ssize_t count = TEMP_FAILURE_RETRY(recv(dSocket, buf, sizeof(buf), 0));
                if (count == -1)
                {
                    //ERROR("Can't take a request");
                    //return -1;
                }
                if (count == 0)
                {
                    //closed connection
                }

                res = http_parser.parse(http_request, buf, buf + sizeof(buf));
                if (res == httpparser::HttpRequestParser::ParsingError)
                {
                    //WARNING("400 Bad Request");
                    //return -2;
                }
                if(res == httpparser::HttpRequestParser::ParsingCompleted)
                {
                    state = ST_RQ_PARSE;
                }
            }
            tTimeout = tTimeout - (time(nullptr) - tParse);
            break;
        }
        case ST_RQ_PARSE:
        {
            if(http_request.method == "GET")
            {
                std::vector<std::string> command = get_commands(http_request.uri);
                if(command[0] == "dispenser")
                {
                    m_pExecutor = new CDevice(cfg->GetPathCOMPort().c_str());
                }
                else
                    m_pExecutor = new CFileReader;
                m_pExecutor->SetURI(command);
                state = ST_EXECUTE_REQ;
            }
            else
                //error
                ERROR("");
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

            response = "HTTP/";
            response += http_request.versionMajor + "." + http_request.versionMinor;
            response += m_pExecutor->GetResType() + " " + m_pExecutor->GetResName() + "/r/n";
            response += "Server: C++ server\r\n";
            response += "Content-length: " + m_pExecutor->GetResp().length();
            response += "/r/n/r/n" + m_pExecutor->GetResp();

            tTimeout = cfg->GetTimeout();
            all_sz = 0;
            state = ST_SEND_RES;
            break;
        }
        case ST_SEND_RES:
        {
            if(tTimeout <= 0)
            {
                ERROR("Timeout");
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

std::vector<std::string> CClient::get_commands(std::string &uri)
{
    std::vector<std::string> resources = split(uri,'/');
    std::vector<std::string> ret_vector;
    for (auto &resource : resources) {
        if (resource.empty())
            continue;
        ret_vector.push_back(resource);
    }
    return ret_vector;
}

void CFileReader::Run(void)
{

}
