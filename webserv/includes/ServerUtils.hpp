#pragma once

#include "SocketManager.hpp"

void    closeSockets(std::vector<int>& pollfd) {
    for (size_t i = 0; i < pollfd.size(); i++)
    {
        close(pollfd[i]);
    }
}