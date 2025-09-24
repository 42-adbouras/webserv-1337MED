#include "../includes/Server.hpp"

Server::Server(Data data) {
    _data = &data;
    // std::cout << data._servers[0]._listen[0].first << std::endl;
    // std::cout << data._servers[0]._listen[0].second << std::endl;
    std::cout << "data parsed successfully." << std::endl;
}

int Server::setNonBlocking(int sockFd) {
    int flags = fcntl(sockFd, F_GETFL);
    flags |= O_NONBLOCK;
    return fcntl(sockFd, F_SETFL, flags);
}

void    Server::initListenSocket(void) {
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd == -1)
        throw ServerExcept(errno);
    _pollFd.push_back(sockFd);
    Server::setNonBlocking(_pollFd[0]); // set listening socket to non-blocking.
}


Server::~Server() {}