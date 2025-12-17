#pragma once

#include  "./Client.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <cerrno>
#include <csignal>

typedef    size_t    wsrv_timer_t;

#include "SocketManager.hpp"
#include "CookiesSessionManager.hpp"
#include <fcntl.h>

struct Data;
class Server
{
    private:
        std::vector<Client>        _client;
        CookiesSessionManager    &_sessionManager;
    public:
        size_t                        _OpenPort;
        Server(CookiesSessionManager& sessionManager, int portOpen);
        std::vector<Client>&    getListOfClients(void);
        void    addClients(Client client, std::vector<struct pollfd> &_pollfd);
        wsrv_timer_t    wsrv_find_next_timeout(void);
        bool            wsrv_timeout_closer(std::vector<struct pollfd>& pollFd);
        Status            readClientRequest(std::vector<struct pollfd>& pollFd, size_t cltIndex, size_t& loopIndex);
        void            responsePart(size_t cltIndex);
        // void            generateResponse()
        void    handleDisconnect(int index, std::vector<struct pollfd>& _pollfd);
        void    closeClientConnection(void);
        ~Server();
        Client&         getClientReqCGI(int pipeFd);
        ClientState    readRequest(size_t cltIndx);
        ClientState    cgiRespond(Client& client);
};

void    CGI_errorResponse(Client& client, int statusCode);
void    signalHandler(int sig);