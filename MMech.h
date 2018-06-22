//
// Created by vadim on 6/7/18.
//

#ifndef SERVER_NEW_MMECH_H
#define SERVER_NEW_MMECH_H


#include <string>
#include <vector>

class CProtocol
{
    int hDevice;
    //bool isMovementCommand(const char &byte);
public:
    CProtocol();
    ~CProtocol();

    void init(const std::string &path);
    int GetDescriptor();
    ssize_t send(const std::vector<uint8_t> &bytes);
    bool recv(std::vector<uint8_t> &buffer, size_t size, time_t seconds = 120, suseconds_t microseconds = 0);
};

#endif //SERVER_NEW_MMECH_H
