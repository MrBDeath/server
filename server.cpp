#include "server.h"

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

bool Server::Start(const Config& cfg)
{

    int fd_socket = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if(fd_socket < 0)
    {
        ERROR("The socket creation failed with error %d", errno);
        return false;
    }
    MiniMech.init(cfg.GetPathCOMPort().c_str());


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


    while(true)
    {
        int client_socket;
        client_socket = (int)TEMP_FAILURE_RETRY(accept4(fd_socket, nullptr, nullptr, SOCK_NONBLOCK));
        if(client_socket < 0) {
            if (!IS_EAGAIN())
                return false;
        }
        else
            client.emplace_back(client_socket);

        for(auto it = client.begin(); it != client.end(); ++it)
        {
            bool isReady = it->Run();
            if(isReady)
                client.erase(it);
        }












        /*

        if (client_socket == 0)
        {
            WARNING("Can't accept client socket");
            DEBUG("client socket returned %d", client_socket);
            return false;
        }
        if(cfg.GetForkEnabled())
        {
            pid_t child_process = fork();

            if(child_process < 0)
            {
                ERROR("Can't create a child process");
                return false;
            }
            else if (child_process > 0)
                close(client_socket);
            else
            {
                try
                {
                    make_response(client_socket, cfg);
                }
                catch (...)
                {
                    close(client_socket);
                    exit(EXIT_FAILURE);
                }
            }
        }
        else
        {
            try
            {
                make_response(client_socket, cfg);
            }
            catch (...)
            {
                close(client_socket);
                exit(EXIT_FAILURE);
            }
        }*/
    }
    return true;
}

void Server::make_response(int client_socket, const Config& cfg)
{
    struct
    {
        int err_type;
        std::string err_name;
    } res_s;

    httpparser::Request http_request;

    std::stringstream response;
    std::stringstream response_body;

    int ret_response = parsing(client_socket,http_request,cfg);

    switch(ret_response)
    {
        case -1:
        {
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
        case 1:
        {
            res_s.err_type = 504;
            res_s.err_name = "Gateway Timeout";
            break;
        }
        case 2:
        {
            res_s.err_type = 400;
            res_s.err_name = "Bad Request";
            break;
        }
        default: break;
    }
    response << "HTTP/" << http_request.versionMajor << '.' << http_request.versionMinor << " ";


    if (http_request.method == "GET")
    {
        std::vector <std::string> command = get_commands(http_request.uri);
        if(command[0] == "dispenser")
        {
            res_s.err_type = 400, res_s.err_name = "Bad Request";
            std::vector<uint8_t> bResponse;
            std::vector<uint8_t> bRequest;
            std::string json;
            if(command.size() > 1)
            {
                if (command[1] == "status")
                {
                    res_s.err_type = 200, res_s.err_name = "OK";
                    json = MiniMech.GetStatus();
                    response_body << json;
                }
                else if (command[1] == "dispense")
                {
                    if (command.size() > 2 && !command[2].empty()) {
                        res_s.err_type = 200, res_s.err_name = "OK";
                        json = MiniMech.Dispense((uint8_t) strtol(command[2].c_str(), nullptr, 10));
                        response_body << json;
                    }
                }
                else if(command[1] == "learn")
                {
                    json = MiniMech.Learn();
                    res_s.err_type = 200; res_s.err_name = "OK";
                    response_body << json;
                }
                else if(command[1] == "reset")
                {
                    json = MiniMech.Reset();
                    res_s.err_type = 200; res_s.err_name = "OK";
                    response_body << json;
                }
            }
        }
        else
        {
            std::ifstream n_file(cfg.GetResourcePath() + http_request.uri,
                                 std::ifstream::binary | std::ifstream::in);
            if (!n_file)
            {
                res_s.err_type = 404, res_s.err_name = "Not Found";
                WARNING("404 Not Found");
            }
            else
            {
                res_s.err_type = 200, res_s.err_name = "OK";
                response_body << n_file.rdbuf();
                n_file.close();

            }
        }
    }
    else
    {
        res_s.err_type = 501, res_s.err_name = "Not Implemented";
        WARNING("501 Not Implemented");
    }

    if (response_body.str().empty())
    {
        std::ifstream n_file(cfg.GetErrorHTMLPath() + "/" + std::to_string(res_s.err_type) + ".html",
                             std::ifstream::binary | std::ifstream::in);
        if (!n_file)
            n_file.open(cfg.GetErrorHTMLPath() + "/" + cfg.GetStndrdErrorHTML() + ".html",
                        std::ifstream::binary | std::ifstream::in);
        if (!n_file)
            response_body << "<html>\n"
                             "    <head>\n"
                             "        <title>Error</title>\n"
                             "    </head>\n"
                             "    <body>\n"
                             "        <center>\n"
                             "            <h1>Something went wrong</h1>\n"
                             "        </center>\n"
                             "    </body>\n"
                             "</html>";
        else
        {
            response_body << n_file.rdbuf();
            n_file.close();
        }
    }

    response << res_s.err_type << ' ' << res_s.err_name
             << "Server: C++ server\r\n"
             << "Content-length: " << response_body.str().length()
             << "\r\n\r\n" << response_body.str();

    ssize_t s = 0;
    size_t q = response.str().length(), all_sz = 0;

    struct timeval tv{};
    tv.tv_sec = cfg.GetTimeout();
    tv.tv_usec = 0;


    do {
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(client_socket,&wfds);
        long int retval = TEMP_FAILURE_RETRY(select(FD_SETSIZE,nullptr,&wfds,nullptr,&tv));

        if(retval < 0)
        {
            ERROR("select() returned with error %d", errno);
            break;
        }
        else if(retval  == 0)
        {
            WARNING("Sending timeout");
            break;
        }
        else if(!FD_ISSET(client_socket,&wfds))
        {
            ERROR("Writing to the file descriptor is forbidden");
            break;
        }

        s = TEMP_FAILURE_RETRY(send(client_socket, response.str().c_str() + all_sz, q - all_sz, 0));

        if (s < 0) break;
        all_sz += s;
    } while (all_sz < q);

    close(client_socket);
    if(cfg.GetForkEnabled()) exit(EXIT_SUCCESS);
}

int Server::parsing(int client_socket, httpparser::Request &http_request, const Config &cfg)
{
    struct timeval tv = {0};
    tv.tv_sec = cfg.GetTimeout();
    tv.tv_usec = 0;

    char buf[4098];
    httpparser::HttpRequestParser http_parser;
    httpparser::HttpRequestParser::ParseResult res;

    do {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(client_socket, &rfds);

        long int retval = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &rfds, nullptr, nullptr, &tv));

        if (retval < 0)
        {
            ERROR("Some problems with select()");
            return -1;
        }

        if (retval == 0 || !FD_ISSET(client_socket, &rfds))
        {
            WARNING("504 Gateway Timeout");
            return 1;
        }

        ssize_t count = TEMP_FAILURE_RETRY(recv(client_socket, buf, sizeof(buf), 0));
        if (count == -1)
        {
            ERROR("Can't take a request");
            return -1;
        }

        res = http_parser.parse(http_request, buf, buf + sizeof(buf));
        if (res == httpparser::HttpRequestParser::ParsingError)
        {
            WARNING("400 Bad Request");
            return -2;
        }

    } while (res == httpparser::HttpRequestParser::ParsingIncompleted);
    return 0;
}