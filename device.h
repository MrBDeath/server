//
// Created by vadim on 6/7/18.
//

#ifndef SERVER_NEW_DEVICE_H

#define SERVER_NEW_DEVICE_H

#include "executor.h"
#include "MMech.h"

#define MULTIMECH_ACK       0x06
#define MULTIMECH_NAK       0x15
#define MULTIMECH_EOT       0x03
#define MULTIMECH_ENDCOM    0x04

#define MULTIMECH_COMMAND_STATUS 0x40
#define MULTIMECH_COMMAND_DISPENSE 0x42




class CDevice : public IExecutor
{

    CProtocol protocol;
    bool transfer(const std::vector<uint8_t> &bytes, std::vector<uint8_t> *response, time_t seconds = 120);
    std::string generateJSON(const std::vector<uint8_t> &bytes);
    virtual void Run();
    uint8_t checksum(const std::vector<uint8_t> &bytes);
public:

    CDevice(const Config &conf);
    ~CDevice();
    bool init(const char *path);
    enum StateMachine
    {
        STATE_SENDING,
        STATE_RECV_HEAD,
        STATE_RECV_BODY,
        STATE_CHECKSUM,
        STATE_ANSWER,
        STATE_SENDING_NAK,
        STATE_SENDING_ACK,
        STATE_RECV_ENDCOM,
        STATE_WAIT,
    };

    StateMachine currentState;

    std::string Dispense(uint8_t amount);
    std::string GetStatus();
    std::string Learn();
    std::string Reset();
};
#endif //SERVER_NEW_DEVICE_H
