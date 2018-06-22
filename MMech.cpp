//
// Created by vadim on 6/7/18.
//

#include <unistd.h>
#include <fcntl.h>
#include "MMech.h"
#include <termios.h>

CProtocol::CProtocol() : hDevice(-1)
{

}

CProtocol::~CProtocol()
{
    if(hDevice > 0)
        close(hDevice);
}

int CProtocol::GetDescriptor()
{
    return hDevice;
}

void CProtocol::init(const std::string &path)
{
    hDevice = open(path.c_str(), O_NOCTTY | O_RDWR);
    if(hDevice < 0)
    {
        // ALARM!!!!!!
        return;
    }

    struct termios options = {0};
    tcgetattr(hDevice, &options);

    cfmakeraw(&options);

    options.c_cflag |= CREAD;
    options.c_cflag |= PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;

    cfsetispeed(&options,B4800);
    cfsetospeed(&options,B4800);

    tcsetattr(hDevice, TCSANOW, &options);
}

ssize_t CProtocol::send(const std::vector<uint8_t> &bytes)
{
    ssize_t retval = 0;
    do
    {
        ssize_t szWrite = TEMP_FAILURE_RETRY(write(hDevice, bytes.data() + retval, bytes.size() - retval));
        if(szWrite < 0)
        {
            return -1;
        }
        retval += szWrite;
    } while(retval < bytes.size());
    return retval;
}

bool CProtocol::recv(std::vector<uint8_t> &buffer, size_t size, time_t seconds, suseconds_t microseconds)
{
    timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = microseconds;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(hDevice, &rfds);

    size_t retval = 0;
    buffer.resize(size);

    do
    {
        fd_set fds = rfds;
        ssize_t szSelect = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &fds, nullptr, nullptr, &tv));
        if (szSelect <= 0)
        {
            buffer.resize(retval);
            return false;
        }
        ssize_t szRead;
        szRead = TEMP_FAILURE_RETRY(read(hDevice, buffer.data() + retval, size - retval));
        if(szRead < 0)
        {
            buffer.resize(retval);
            return false;
        }
        retval += szRead;
    } while(retval < size);
    return true;
}