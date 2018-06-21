#ifndef SERVER_H__
#define SERVER_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <netdb.h>
#include <fstream>
#include "config.h"
#include "httpparser/request.h"
#include "httpparser/httprequestparser.h"
#include "httpparser/response.h"
#include "httpparser/httpresponseparser.h"
#include "debug.h"
#include "log.h"
#include <csignal>
#include <sys/wait.h>
#include "client.h"
#include "device.h"
#include <climits>

class Server
{
public:

    Server() = default;
    ~Server() = default;

    bool Start(Config& cfg);


private:
    std::vector<CClient> client;
    sockaddr_in st_sock{};
    /*void make_response(int client_socket, const Config& cfg);
    //std::vector<std::string> get_commands(std::string &uri);
    int parsing(int client_socket, httpparser::Request &http_request, const Config &cfg);*/
};

#endif