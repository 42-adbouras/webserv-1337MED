#include "Client.hpp"

Client::Client(int fd, const serverBlockHint& server_block) : _fd(fd), _serverBlockHint(server_block) {
    // std::cout << "client connected" << std::endl;
}

int Client::getFd() const {
    return _fd;
}
Client::~Client() {}

ClientState    Client::getStatus() const {
    return _clientState;
}
void    Client::setClientState(ClientState clientState) {
    _clientState = clientState;
}
void	Client::setRequest( Request req ) {
	_request = req;
}
Request& Client::getRequest() {
	return _request;
}

void    Client::setTimeOut(std::time_t current) {
    this->_startTimeOut = current;
}
