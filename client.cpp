//
// Created by vadim on 6/18/18.
//

#include "client.h"

CClient::CClient(int fd) : fdesc(fd) {}


std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

bool CClient::Run()
{
    switch (state)
    {
        case ST_PARSE:
        {
            break;
        }
        case ST_RQ_PARSE:
        {
            if (dispenser)
                m_pExecutor = new CDevice;
            else
                m_pExecutor = new CFileReader;
            break;
        }
        case ST_EXECUTE_REQ:
        {
            m_pExecutor->Run();
            break;
        }
    }
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
