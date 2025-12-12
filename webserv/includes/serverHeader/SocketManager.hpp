#pragma once

// #include <iostream>
#include <netdb.h>
#include <poll.h>
#include <cstring>
#include <cerrno>
#include <vector>
// #include <signal.h>
#include "../Config.hpp"
// #include "Client.hpp"

struct  TableOfListen;
struct  ServerEntry;
class   Server;
struct  Data;
class   Client;

typedef std::vector<std::pair<TableOfListen*, ServerEntry*> >   serverBlockHint;

class   SocketManager {
    private:
        Data                                            *_config;
        std::vector<TableOfListen>                      &_tableOfListen;
        // std::vector<std::pair<int, struct sockaddr*> >    _listenSocks;
        void    bindSockets(size_t counter);
    public:
        SocketManager(Data& config, std::vector<TableOfListen>& tableOfListen);
        void    setTableOfListen(std::vector<TableOfListen>& table);
        Status  PollingForEvents(std::vector<struct pollfd>& pollFd, Server& server,size_t cltSize);    
        void    initSockets(void);
        void    listenToPorts(void) ;
        void    runCoreLoop(void);
        void    setListenEvent(std::vector<struct pollfd>& _pollfd);
        bool    checkForNewClients( std::vector<struct pollfd>& _pollfd, Server& _server );
        void    hanldVirtualHost(TableOfListen& table, size_t index);
        bool    checkIfAlreadyBinded(size_t index) const;
        void    closeListenSockets(void) const;
        void    closeClientsSockets();
        serverBlockHint   detectServerBlock(int sockFd) const;
        void    rmClientFromPoll(std::vector<struct pollfd>& _pollfd, size_t  cltSize);
        // std::vector<ServerEntry>&    retrieveServerBlock(size_t index);
        // ----- Utils -----
        // static struct sockaddr_in  getSockaddr(void);
        static int  setNonBlocking(int fd);
        size_t      portCounter(void) const;
        void        handlErrCloses(std::vector<struct pollfd>& _pollfd, Server& server , size_t cltSize);
        // ---------------  CGI     ---------------------

        void    readFromCgi(std::vector<Client>& clients, std::vector<struct pollfd>& pollFd, Server& srvr, size_t* coreIndex);
        void    cgiEventsChecking(std::vector<Client>& clients, std::vector<struct pollfd>& pollFd, Server& srvr);
        bool    isCgiRequest(std::vector<struct pollfd>& pollFd, Client& client, size_t index);
        // void    signalHandler(int sig) {
        //     std::cout << "Signal is: " << sig  << std::endl;
        //     exit(EXIT_SUCCESS);
        // };
        ~SocketManager(){};
};

struct  InterfaceState
{
    bool    alreadyBinded;
    int     fd;
};

struct TableOfListen
{
    int                 _fd;
    struct sockaddr_storage addr; // own the address bytes (no dangling pointer)
    socklen_t           addr_len;
    std::string         _ip;
    std::string         _port;
    std::string         _serverName;
    unsigned int        _serverBlockId;
    InterfaceState      _interfaceState;

    bool    operator==(const TableOfListen& other) const
    {
        str localhost = str("localhost");
        str digitHost = str("127.0.0.1");

        if (_ip == other._ip && _port == other._port) {

            return true;
        }
        else if (_ip != other._ip && _port == other._port)
        {
            if ((_ip == localhost || _ip == digitHost) && (other._ip == localhost || other._ip == digitHost))
                return true;
            std::string anyAddr("0.0.0.0");

            if ((_ip == anyAddr || other._ip == anyAddr) && (_ip == localhost || _ip == digitHost || other._ip == localhost || other._ip == digitHost))
                return true;
        }
        return false;
    }
};
void	displayHashTable(const std::vector<TableOfListen> &table);
void    displayPOllList(const std::vector<pollfd>& list);

