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

CGIContext&  Client::getCgiContext(void) const {
    return *_cgiContext;
}

void    Client::setCgiContext() {
    _cgiContext->_path = "www/cgi-scripts/hello.py";
    _cgiContext->_name = "hello.py"; _cgiContext->_ntrp = "/usr/bin/python3";
    _cgiContext->_method = "POST"; _cgiContext->_serverName = "ait-server";
    _cgiContext->_serverAddr = "0.0.0.0"; _cgiContext->_serverPort = "8080";
    _cgiContext->_contenType = "text/palin";
    _cgiContext->_query["name"] = "world"; _cgiContext->_query["lang"] = "en";
    _cgiContext->_headers["Host"] = "0.0.0.0:8080";
    _cgiContext->_headers["Content-Type"] = "text/plain";
    _cgiContext->_headers["User-Agent"] = "webserv-dev/0.1";
    _cgiContext->_headers["Accept"] = "*/*";
    _cgiContext->_body = "<user>\n\t"
                "<name>Jane Doe</name>\n\t"
                "<email>jane.doe@example.com</email>\n\t"
                "<age>25</age>\n"
                "</user>";
}