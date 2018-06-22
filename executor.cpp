//
// Created by vadim on 6/21/18.
//

#include "executor.h"

IExecutor::IExecutor(const Config &conf) :
cfg(conf)
{

}

void IExecutor::SetURI(const std::string &commands)
{
    uri = commands;
}

std::string IExecutor::GetResp()
{
    return response;
}

std::string IExecutor::GetResName() {
    return stRes.Name;
}
int IExecutor::GetResType()
{
    return stRes.Type;
}
