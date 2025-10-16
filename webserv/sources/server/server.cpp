#include "../includes/serverHeader/Server.hpp"

Server::Server() {
    std::cout << "server start ..." << std::endl;
}

void    Server::addClients(int clientFd, std::vector<struct pollfd> &_pollfd) {
    struct pollfd   temp;
    temp.fd = clientFd;
    temp.events = POLLIN | POLLOUT;
    _clientSocks.push_back(clientFd);
    _pollfd.push_back(temp);
    std::cout << "client with fd: " << clientFd << " connected" << std::endl;
}

void    Server::handleDisconnect(int index, std::vector<struct pollfd>& _pollfd) {
    close(_pollfd[index].fd);
    _pollfd.erase(_pollfd.begin() + index);
    std::cout << "client fd " << _pollfd[index].fd << " disconnect" << std::endl;

}

// status    Server::statOfUser(int clFd) const {
//     return DISCONNECT
// }

Server::~Server() {}