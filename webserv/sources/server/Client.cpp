#include "../../includes/serverHeader/Server.hpp"
#include "../../includes/serverHeader/Client.hpp"
Client::Client(int fd, const serverBlockHint& server_block) : _fd(fd), _expectedBodyLength(0), _isChunked(false), _serverBlockHint(server_block),
				_cgiProc(CGIProc()), _state(PARSING_HEADERS)
				, _uploadFd(-1)
				, _uploadPath(), _uploadedBytes(0) {
		_sendInfo.fd = -1;
}

int Client::getFd() const {
    return _fd;
}
Client::~Client() {
	_sendInfo.buff.clear();
	_serverBlockHint.clear();
}

ClientState    Client::getStatus() const {
    return _clientState;
}

void    Client::setClientState(ClientState clientState) {
    _clientState = clientState;
}

void	Client::setRequest( Request& req ) {
	_request = req;
}
void	Client::setResponse( Response& res ) {
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
/*	Client Cgi Status	*/
void        Client::setCltCgiState(ClientCGIState cltCgiState) {
	_cltCgiState = cltCgiState;
}

ClientCGIState  Client::getCltCgiState() const {
	return _cltCgiState;
}


void	Client::setCgiContext(CGIContext& cgiContext) {
	this->_cgiContext = cgiContext;
}

void Client::setExpectedBodyLength( size_t lgth ) {
	_expectedBodyLength = lgth;
}
void Client::setIsChunked( bool chunked ) {
	_isChunked = chunked;
}
void Client::setLeftover( str& leftover ) {
	_leftover = leftover;
}

size_t Client::getExpectedBodyLength( void ) const { return _expectedBodyLength; }
bool Client::getIsChunked( void ) const { return _isChunked; }
str& Client::getLeftover( void ) { return _leftover; }
