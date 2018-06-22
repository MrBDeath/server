//
// Created by vadim on 6/22/18.
//

#ifndef SERVER_NEW_UTILS_H
#define SERVER_NEW_UTILS_H

#include <string>
#include <vector>

namespace Utils
{
    std::vector<std::string> split(const std::string &s, char delimiter);
    std::vector<std::string> get_commands(const std::string &uri);
} // namespace Utils

#endif //SERVER_NEW_UTILS_H
