//
// Created by vadim on 6/7/18.
//

#include "device.h"
// -------------------------------------

CDevice::CDevice() : currentState(STATE_SENDING)
{

}

CDevice::CDevice(const char* path) : currentState(STATE_SENDING)
{
    protocol.init(path);
}

CDevice::~CDevice()
= default;

// --------------------------------------

bool CDevice::transfer(const std::vector<uint8_t> &bytes, std::vector<uint8_t> *response, time_t seconds)
{
    int err_sending_counter = 0, err_recove_counter = 0;
    bool recove_error = false;
    currentState = STATE_SENDING;

    while (err_sending_counter < 5 && err_recove_counter < 5)
    {
        switch (currentState)
        {
            case STATE_SENDING:
            {
                if (protocol.send(bytes) == bytes.size())
                    currentState = STATE_ANSWER;
                else
                    return false;
                break;
            }
            case STATE_WAIT:
            {
                timespec tv;
                tv.tv_sec = 0;
                tv.tv_nsec = 200000000;
                TEMP_FAILURE_RETRY(nanosleep(&tv, &tv));
                currentState = STATE_SENDING;
                break;
            }
            case STATE_ANSWER:
            {
                std::vector<uint8_t> buffer;

                do
                {
                    bool status = protocol.recv(buffer, 1, 2);
                    if (!status)
                        return false;
                    if(buffer[0] == MULTIMECH_ACK)
                        if(response)
                            currentState = STATE_RECV_HEAD;
                        else
                            currentState = STATE_SENDING_ACK;//sending ACK

                    else if(buffer[0] == MULTIMECH_NAK)
                    {
                        currentState = STATE_WAIT;
                        ++err_sending_counter;
                    }
                } while(buffer[0] != MULTIMECH_ACK && buffer[0] != MULTIMECH_NAK);
                break;
            }
            case STATE_SENDING_NAK:
            {
                if (protocol.send({MULTIMECH_NAK}))
                    currentState = STATE_RECV_HEAD;
                else
                    err_sending_counter++;
                break;
            }
            case STATE_RECV_HEAD:
            {
                if (!protocol.recv(*response, 4, seconds))
                    recove_error = true;
                currentState = STATE_RECV_BODY;
                break;
            }
            case STATE_RECV_BODY:
            {
                do {
                    std::vector<uint8_t> buf;
                    if (protocol.recv(buf, 1, 2))
                    {
                        response->push_back(buf[0]);
                        if(buf[0] == MULTIMECH_EOT && !recove_error)
                        {
                            currentState = STATE_CHECKSUM;
                            break;
                        }
                    }
                    else
                    {
                        currentState = STATE_SENDING_NAK;
                        break;
                    }
                } while (true);
            }
            case STATE_CHECKSUM:
            {
                std::vector<uint8_t> buf;
                if (protocol.recv(buf, 1, 2))
                {
                    uint8_t check_sum = checksum(*response);
                    response->push_back(buf[0]);

                    if (response->back() != check_sum)
                        currentState = STATE_SENDING_NAK;
                    else
                        currentState = STATE_SENDING_ACK;
                }
                else
                    currentState = STATE_SENDING_NAK;

                break;
            }
            case STATE_SENDING_ACK:
            {
                protocol.send({MULTIMECH_ACK});
                currentState = STATE_RECV_ENDCOM;
                break;
            }
            case STATE_RECV_ENDCOM:
            {
                std::vector<uint8_t> buf;
                if (protocol.recv(buf, 1, 2))
                    if(buf[0] == MULTIMECH_ENDCOM)
                        return true;
                TEMP_FAILURE_RETRY(sleep(10));
                return true;
            }
            default:
                break;
        }
    }
    return false;
}

std::string CDevice::generateJSON(const std::vector<uint8_t> &bytes)
{
    uint8_t command = bytes[3];
    nlohmann::json json;
    switch(command)
    {
        case MULTIMECH_COMMAND_STATUS:
        {
            json["Command"] = "STATUS";
            json["Information"]["FeedSensorBlocked"] = bool(bytes[4] & (1 << 0));
            json["Information"]["ExitSensorBlocked"] = bool(bytes[4] & (1 << 1));
            json["Information"]["ResetSinceLastStatusMessage"] = bool(bytes[4] & (1 << 3));
            json["Information"]["TimingWheelSensorBlocked"] = bool(bytes[4] & (1 << 4));
            json["Information"]["CalibratingDoubleDetect"] = bool(bytes[5] & (1 << 4));
            json["Information"]["THICKNESS"] = bytes[6] - 0x20;
            json["Information"]["LENGTH"] = bytes[7] - 0x20;
            break;
        }
        case MULTIMECH_COMMAND_DISPENSE:
        {
            json["Command"] = "DISPENSE";
            json["Information"]["NumberOfNotesPastExit"] = bytes[5] - 0x20;
            json["Information"]["NumberOfRejectEvents"] = bytes[6] - 0x20;
            break;
        }
        default:
            break;
    }
    return json.dump(4);
}

bool CDevice::init(const char *path)
{
    protocol.init(path);
    return false;
}

uint8_t CDevice::checksum(const std::vector<uint8_t> &bytes)
{
    uint8_t checksum = 0;
    for(size_t i = 0; i < bytes.size(); ++i)
        checksum ^= bytes[i];
    return checksum;
}

std::string CDevice::GetStatus() {
    std::vector<uint8_t> response, request;
    request = {0x04,0x30,0x02,0x40,0x03};
    request.push_back(checksum(request));
    std::string json;
    if(transfer(request,&response,10))
        json = generateJSON(response);
    else
        json = "Error";
    return json;
}

std::string CDevice::Dispense(uint8_t amount)
{
    std::string json;
    std::vector<uint8_t> response, request;
    if(amount > 0 && amount <= 50)
        amount = static_cast<uint8_t>(amount + 0x20);
        request = {0x04,0x30,0x02,0x42,amount,0x03};
        request.push_back(checksum(request));
        if(transfer(request,&response,120))
            json = generateJSON(response);
        else
            json = "Error";
    return json;
}

std::string CDevice::Learn()
{
    std::string json;
    std::vector<uint8_t> response;

    std::vector<uint8_t> request = {0x04,0x30,0x02,0x57,0x44,0x2F,0x33,0x39,0x32,0x2F,0x30,0x03};
    request.push_back(checksum(request));
    if(!transfer(request,&response,10)) {
        json = "Error";
        return json;
    }
    else
    {
        if(response[3] != 0x30)
        {
            json = "Error";
            return json;
        }
    }
    request = {0x04,0x30,0x02,0x42,0x21,0x03};
    request.push_back(checksum(request));
    if(transfer(request,&response,120))
        json = generateJSON(response);
    else
        json = "Error";
    return json;
}

std::string CDevice::Reset()
{
    std::vector<uint8_t> request = {0x04,0x30,0x02,0x44,0x03};
    request.push_back(checksum(request));
    nlohmann::json json;
    json["Command"] = "RESET";
    if(transfer(request,nullptr,10))
        json["STATUS"] = "success";
    else
        json["STATUS"] = "fail";
    return json.dump(4);
}

void CDevice::Run()
{
    std::string json = "Error";
    stRes.Name = "Bad Request";
    stRes.Type = 400;
    if(command.size() > 1)
    {
        if(command[1] == "status")
        {
            json = GetStatus();
            stRes.Name = "OK";
            stRes.Type = 200;
        }
        if(command[1] == "learn")
        {
            json = Learn();
            stRes.Name = "OK";
            stRes.Type = 200;
        }
        if(command[1] == "reset")
        {
            json = Reset();
            stRes.Name = "OK";
            stRes.Type = 200;
        }
        if(command[1] == "dispense")
            if(command.size() > 2 && !command[2].empty())
            {
                json = Dispense((uint8_t) strtol(command[2].c_str(), nullptr, 10));
                stRes.Name = "OK";
                stRes.Type = 200;
            }
    }
    response = json;
}

