#pragma once

#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <cstring>
#include <cerrno>
#include <vector>

struct  TableOfListen;
struct  ServerEntry;
typedef std::vector<std::pair<TableOfListen*, ServerEntry*> >   serverBlockHint;
// #include <utility>
// #include "Server.hpp"
#include "../Config.hpp"
class   Server;

struct  Data;


class   SocketManager {
    private:
        Data                                            *_config;
        std::vector<TableOfListen>                      &_tableOfListen;
        // std::vector<std::pair<int, struct sockaddr*> >    _listenSocks;
        void    bindSockets(size_t counter);
    public:
        SocketManager(Data& config, std::vector<TableOfListen>& tableOfListen);
        void    setTableOfListen(std::vector<TableOfListen>& table);
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
        // std::vector<ServerEntry>&    retrieveServerBlock(size_t index);
        // ----- Utils -----
        // static struct sockaddr_in  getSockaddr(void);
        static int  setNonBlocking(int fd);
        size_t      portCounter(void) const;
        ~SocketManager(){};
};

struct TableOfListen
{
    int _fd;
    struct sockaddr*    addr;
    std::string _ip;
    std::string _port;
    std::string _serverName;
    unsigned int    _serverBlockId;
    bool    alreadyBinded;
    bool    operator==(const TableOfListen& other) const {
        if (_ip == other._ip && _port == other._port)
            return true;
        else if (_ip != other._ip && _port == other._port)
        {
            std::string anyAddr("0.0.0.0");
            if (_ip == anyAddr || other._ip == anyAddr)
                return true;
        }
        return false;
    }
};
void	displayHashTable(const std::vector<TableOfListen> &table);