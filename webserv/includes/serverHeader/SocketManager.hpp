#pragma once

#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <cstring>
#include <cerrno>

// #include <utility>
#include "../Config.hpp"

#include "Server.hpp"

struct Data;

class   SocketManager {
    private:
        Data                *_config;
        std::vector<std::pair<int, struct sockaddr*> >    _listenSocks;
        void    bindSockets(void);
    public:
        SocketManager(Data& config);
        void    initSockets(void);
        void    listenPorts(void) ;
        void    acceptIncomingConn(void);
        // std::vector<ServerEntry>&    retrieveServerBlock(size_t index);
        // ----- Utils -----
        // static struct sockaddr_in  getSockaddr(void);
        static int  setNonBlocking(int fd);
        size_t      portCounter(void) const;
        ~SocketManager(){};
};

