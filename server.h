#ifndef SERVER_H__
#define SERVER_H__


#include <netinet/in.h>
#include "config.h"
#include "client.h"
#include <memory>

class Server
{
public:

    Server() = default;
    ~Server() = default;

    bool Start(Config& cfg);


private:
    std::vector<std::unique_ptr<CClient>> client;
    sockaddr_in st_sock{};
    /*void make_response(int client_socket, const Config& cfg);
    //std::vector<std::string> get_commands(std::string &uri);
    int parsing(int client_socket, httpparser::Request &http_request, const Config &cfg);*/
};

#endif