#pragma once

#include  "./Client.hpp"
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>

typedef size_t    wsrv_timer_t;

#include "SocketManager.hpp"
#include "CookiesSessionManager.hpp"

struct Data;
class Server
{
    private:
        std::vector<Client>        _client;
        CookiesSessionManager    &_sessionManager;
    public:
        size_t					_OpenPort;
        std::vector<Client>&	getListOfClients(void);
        void					addClients(Client client, std::vector<struct pollfd> &_pollfd);
        wsrv_timer_t			wsrv_find_next_timeout(void);
        void					wsrv_timeout_closer(std::vector<struct pollfd>& pollFd);
        void					handleDisconnect(int index, std::vector<struct pollfd>& _pollfd);
        void					closeClientConnection(void);
        Client&					getClientReqCGI(int pipeFd);
        ClientState				readRequest(size_t cltIndx);
        
        Server(CookiesSessionManager& sessionManager, int portOpen);
        ~Server();
};

Connection  CGI_errorResponse(Client& client, int statusCode);
void        signalHandler(int sig);