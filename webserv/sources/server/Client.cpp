#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _status(NON) {
    // std::cout << "client connected" << std::endl;
}

int Client::getFd() const {
    return _fd;
}
Client::~Client() {}

Status    Client::getStatus() const {
    return _status;
}
void    Client::setStatus(Status status) {
    _status = status;
}
