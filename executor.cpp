//
// Created by vadim on 6/21/18.
//

#include "executor.h"


void IExecutor::SetURI(const std::vector<std::string> &commands)
{
    command = commands;
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
