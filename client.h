//
// Created by vadim on 6/18/18.
//

#ifndef SERVER_NEW_CLIENT_H
#define SERVER_NEW_CLIENT_H


#include "executor.h"
#include "httpparser/request.h"

class CClient
{
    enum
    {
        ST_INIT,
        ST_PARSE,
        ST_RQ_PARSE,
        ST_EXECUTE_REQ,
        ST_MAKE_RES,
        ST_SEND_RES,
    } state;

    IExecutor *m_pExecutor;
    int dSocket;
    const Config &cfg;
    time_t tTimeout;
    httpparser::Request http_request;
    std::string response;

    size_t all_sz;

public:
    CClient(int fd, const Config &conf);
    bool Run();
    int getFileDesc();
    ~CClient() {}
};


#endif //SERVER_NEW_CLIENT_H
