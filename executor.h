//
// Created by vadim on 6/21/18.
//

#ifndef SERVER_NEW_EXECUTOR_H
#define SERVER_NEW_EXECUTOR_H


#include <string>
#include <vector>
#include "config.h"

class IExecutor
{
public:
    IExecutor(const Config &conf);
    virtual void Run() = 0;
    std::string GetResp();
    void SetURI(const std::string &commands);
    int GetResType();
    std::string GetResName();

protected:
    const Config &cfg;
    std::string uri;
    std::string response;
    struct
    {
        int Type;
        std::string Name;
    } stRes;
};




#endif //SERVER_NEW_EXECUTOR_H
