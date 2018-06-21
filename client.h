//
// Created by vadim on 6/18/18.
//

#ifndef SERVER_NEW_CLIENT_H
#define SERVER_NEW_CLIENT_H

#include <vector>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include "device.h"
#include "httpparser/request.h"
#include "httpparser/httprequestparser.h"
#include "config.h"
#include "executor.h"

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
    Config *cfg;
    time_t tTimeout;
    httpparser::Request http_request;
    std::string response;

    size_t all_sz;

public:
    CClient(int fd, Config &conf);
    bool Run();
    int getFileDesc();
    ~CClient() {}
    std::vector<std::string> get_commands(std::string &uri);
};


class CFileReader : public IExecutor
{
public:
    void Run();

};

#endif //SERVER_NEW_CLIENT_H
