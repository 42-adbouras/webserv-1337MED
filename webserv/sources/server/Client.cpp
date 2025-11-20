#include "Client.hpp"

Client::Client(int fd, const serverBlockHint& server_block) : _fd(fd), _serverBlockHint(server_block), _cgiProc(CGIProc()) {
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
void	Client::setResponse( Response res ) {
	_response = res;
}

Request& Client::getRequest() {
	return _request;
}
Response& Client::getResponse() {
	return _response;
}

void    Client::setTimeOut(std::time_t timeout) {
    this->_timeOut = timeout;
}

std::time_t Client::getTimeOut(void) const {
    return this->_timeOut;
}

void        Client::setStartTime(std::time_t start) {
    this->_startTime = start;
    // std::cout << CYAN << "START TIME IS= " << _startTime << RESET << std::endl;
}

std::time_t   Client::getStartTime(void) const {
    return this->_startTime;
}

void    Client::setRemainingTime(wsrv_timer_t  remaining) {
    _remaining = remaining;
}

wsrv_timer_t    Client::getRemainingTime(void) const {
    return _remaining;
}

const CGIContext&  Client::getCgiContext(void) const {
	return _cgiContext;
}

void	Client::setCgiContext(CGIContext& cgiContext) {
	this->_cgiContext = cgiContext;
}