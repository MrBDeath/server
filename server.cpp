#include <wait.h>
#include <cstring>
#include "server.h"
#include "debug.h"
#include "log.h"
#include <arpa/inet.h>
#include <unistd.h>

void clean_up(int)
{
    wait4(0, nullptr, WNOHANG, nullptr);
    DEBUG("Child process was cleaned");
}

#if EAGAIN == EWOULDBLOCK
#define IS_EAGAIN() ((errno) == EAGAIN)
#else
#define IS_EAGAIN() ((errno) == EAGAIN || (errno) == EWOULDBLOCK)
#endif

bool Server::Start(Config& cfg)
{

    int fd_socket = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if(fd_socket < 0)
    {
        ERROR("The socket creation failed with error %d", errno);
        return false;
    }

    int temp = 1;
    if(setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(temp)) < 0)
    {
        ERROR("Can't set new socket options. Error: %s", strerror(errno));
        return false;
    }

    memset(&st_sock,0,sizeof(st_sock));
    st_sock.sin_family = PF_INET;
    st_sock.sin_port = htons((in_port_t)strtol(cfg.GetHostPort().c_str(),nullptr,10));
    inet_aton(cfg.GetHostName().c_str(),&st_sock.sin_addr);

    int bnd = bind(fd_socket, (struct sockaddr*) &st_sock, sizeof(st_sock));
    if(bnd < 0)
    {
        close(fd_socket);
        ERROR("Socket binding ended with error %d", errno);
        return false;
    }

    if(listen(fd_socket,100) < 0)
    {
        ERROR("Could not start listening socket with error %d", errno);
        close(fd_socket);
        return false;
    }

    LOG("----------------SERVER---------------");
    INFO("Server was started successfully");
    LOG("-------------------------------------");


    struct sigaction sigchld_action = {};
    memset(&sigchld_action, 0, sizeof(sigchld_action));
    sigchld_action.sa_handler = &clean_up;
    sigaction(SIGCHLD, &sigchld_action, nullptr);


    while(true) {
        int client_socket;
        client_socket = (int) TEMP_FAILURE_RETRY(accept4(fd_socket, nullptr, nullptr, SOCK_NONBLOCK));
        if (client_socket < 0) {
            if (!IS_EAGAIN())
                return false;
        }
        else
        {
            client.emplace_back(new CClient(client_socket, cfg));
        }

        for (auto it = client.begin(); it != client.end(); ++it) {
            bool isReady = (*it)->Run();
            if (isReady)
            {
                close((*it)->getFileDesc());
                client.erase(it);
                --it;
            }
        }
    }
}