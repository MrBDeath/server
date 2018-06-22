//
// Created by vadim on 6/22/18.
//

#include <sstream>
#include "utils.h"

std::vector<std::string> Utils::split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
        tokens.push_back(token);
    return tokens;
}

std::vector<std::string> Utils::get_commands(const std::string &uri)
{
    std::vector<std::string> resources = Utils::split(uri,'/');
    std::vector<std::string> ret_vector;
    for (auto &resource : resources) {
        if (resource.empty())
            continue;
        ret_vector.push_back(resource);
    }
    return ret_vector;
}