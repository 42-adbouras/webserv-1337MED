#include "../includes/serverHeader/Server.hpp"

Server::Server() {
    std::cout << "server start ..." << std::endl;
}

void    Server::addClients(int clientFd) {
    (void)clientFd;
}

Server::~Server() {}