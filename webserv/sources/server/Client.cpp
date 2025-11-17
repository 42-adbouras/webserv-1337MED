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

CGIContext  Client::getCgiContext(void) const {
    return _cgiContext;
}

void    Client::setCgiContext() {
    _cgiContext._path = "www/cgi-scripts/hello.py";
    _cgiContext._name = "hello.py"; _cgiContext._ntrp = "/usr/bin/python3";
    _cgiContext._method = "POST"; _cgiContext._serverName = "ait-server";
    _cgiContext._serverAddr = "0.0.0.0"; _cgiContext._serverPort = "8080";
    _cgiContext._contenType = "text/palin";
    _cgiContext._query["name"] = "world"; _cgiContext._query["lang"] = "en";
    _cgiContext._headers["Host"] = "0.0.0.0:8080";
    _cgiContext._headers["Content-Type"] = "text/plain";
    _cgiContext._headers["User-Agent"] = "webserv-dev/0.1";
    _cgiContext._headers["Accept"] = "*/*";
    _cgiContext._body = "1234";
}

// ______________________ hello.py ______________________

// CONTENT_LENGTH=4

// CONTENT_TYPE=text/palin

// GATEWAY_INTERFACE=CGI/1.1

// HTTP_ACCEPT=*/*

// HTTP_HOST=0.0.0.0:8080

// HTTP_USER_AGENT=webserv-dev/0.1

// LC_CTYPE=C.UTF-8

// QUERY_STRING=lang=en&name=world

// REQUEST_METHOD=POST

// SCRIPT_FILENAME=hello.py

// SERVER_PROTOCOL=HTTP/1.1

// SERVER_SOFTWARE=ait-server/0.1

// ______________________ BODY (raw) ______________________
// 1234