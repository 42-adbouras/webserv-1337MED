#pragma once

#include <iostream>

#include "Config.hpp"
#include "Server.hpp"

struct Data;

class   SocketManager {
    private:
        Data    *_config;
        std::vector<int>    _pollfd;
    public:
        SocketManager(Data& config);
        void    initSockets(void);

        // ----- Utils -----
        size_t portCounter(void) const;
        ~SocketManager(){};
};