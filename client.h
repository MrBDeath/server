//
// Created by vadim on 6/18/18.
//

#ifndef SERVER_NEW_CLIENT_H
#define SERVER_NEW_CLIENT_H

#include <vector>
#include <sstream>
#include <string>

class IExecutor
{
public:
    virtual void Run(void) = 0;
    virtual void GetResp(void) = 0;
};

class CClient
{
    enum
    {
        ST_PARSE,
        ST_RQ_PARSE,
        ST_EXECUTE_REQ,

    } state;
    IExecutor *m_pExecutor;
    int fdesc;
public:
    explicit CClient(int fd);
    bool Run();
    std::vector<std::string> get_commands(std::string &uri);
    ~CClient() = default;
};


class CFileReader : public IExecutor
{
public:
    virtual void Run(void);

};

class CDevice : public IExecutor
{
public:
    virtual void Run(void);
};

#endif //SERVER_NEW_CLIENT_H
