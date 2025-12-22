#include "../../includes/serverHeader/Server.hpp"
#include "../../includes/serverHeader/Client.hpp"
#include "../../includes/serverHeader/ServerUtils.hpp"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <vector>

Server::Server(CookiesSessionManager& sessionManager, int portOpen) : _sessionManager(sessionManager), _OpenPort(portOpen) {
    (void) _sessionManager;
    g_console.log(SERVER, str("Server started — config=config.conf ..."), BG_CYAN);
}

std::vector<Client>& Server::getListOfClients(void) {
    return _client;
}

bool    Server::wsrv_timeout_closer(std::vector<struct pollfd>& pollFd) {
    bool state = false;

    for (size_t i = 0; i < _client.size(); i++)
    {
        if (std::time(NULL) - _client[i].getStartTime() >= _client[i].getTimeOut())
        {
            std::cout << TIME_OUT << BG_RED << "User FD=" << _client[i].getFd() << " Hors-Ligne!" << RESET << std::endl;
            state = true;
            handleDisconnect(i, pollFd);
            i--;
        }
    }
    if (!state)
        std::cout << INFO << BG_GREEN << "All Clients en-ligne" << RESET << std::endl;
    return state;
}

wsrv_timer_t Server::wsrv_find_next_timeout(void) {
    wsrv_timer_t now = std::time(NULL);
    wsrv_timer_t lower;

    for (size_t i = 0; i < _client.size(); ++i) {
        wsrv_timer_t elapsed = (now - _client[i].getStartTime());
        wsrv_timer_t timeout = _client[i].getTimeOut();
        ssize_t remaining = timeout - elapsed;

        if (remaining <= 0)
            return 0; // already timed out
        _client[i].setRemainingTime(remaining);
    }
    lower = _client[0].getRemainingTime();
    for (size_t i = 1; i < _client.size(); i++)
    {
        if (_client[i].getRemainingTime() < lower)
            lower = _client[i].getRemainingTime();
    }
    std::cout << TIME_OUT << "Remaining Time for Waiting events: " << lower << std::endl;
    return (lower);
}

void    Server::addClients(Client client, std::vector<struct pollfd> &_pollfd) {
    struct pollfd   temp;

    client.setStartTime(std::time(NULL));
    client.setTimeOut(DEF_HEADER_TIME_OUT);/* timeOut to wait for the first request */
    client.setClientState(CS_NEW);
    client._alreadyExec = false;
    client._cgiProc._readPipe = -1;
    client._cgiProc._childPid = -1;
    client._reqInfo.reqStatus = CS_NEW;
    client._sendInfo.resStatus = CS_NEW;
    temp.fd = client.getFd();
    temp.events = POLLIN;
    temp.revents = 0;
    _client.push_back(client);
/**
 *  My Pollfd Layout is [listen sockets] [clinets][Cgi Pipes];
 *  So here i add new clinet after [listen sockets + clinet.size()-1]. (in the middle) 
 */
    _pollfd.insert(_pollfd.begin() + _OpenPort + (_client.size() - 1), temp);
}


ClientState Server::readRequest(size_t cltIndx) {
    // char    buff[SRV_READ_BUFFER];
	std::vector<char>	buff(SRV_READ_BUFFER);
    ssize_t rByte;
    if (_client[cltIndx].getStatus() == CS_NEW) {
		Request request;
        _client[cltIndx].setRequest(request);
    }
    // Request req = _client[cltIndx].getRequest();
    rByte = recv(_client[cltIndx].getFd(), buff.data(), SRV_READ_BUFFER, 0);
    if (rByte > 0)
    {
        /**
         * Read request chunks-chunks, every chunk past to parseRequest,
         * the parse of request must detect if the request is finished by setting ReqInfo to `CS_READING_DONE`
         * else `CS_READING`
         * */
        // _client[cltIndx].getRequest().setBuffer(buffer);
        // _client[cltIndx]._reqInfo.buffer.insert(_client[cltIndx]._reqInfo.buffer.end(), buff, buff + rByte);
		_client[cltIndx]._reqInfo.buffer.insert(_client[cltIndx]._reqInfo.buffer.end(), buff.data(), buff.data() + rByte);
        requestHandler(_client[cltIndx]);
        if (_client[cltIndx]._reqInfo.reqStatus == CS_READING_DONE) {
            _client[cltIndx]._sendInfo.resStatus = CS_START_SEND; /* To track first try of send-response */
            return CS_READING_DONE;
        }
        return CS_READING;
    }
    else if (rByte == 0)
    {
        return CS_DISCONNECT;
    }
    else
    {
        std::cout << "Read Request: " << strerror(errno) << std::endl;
        return CS_READING;
    }
    return CS_FATAL;
}


// ClientState Server::readRequest(size_t cltIndx) {
//     char    buff[SRV_READ_BUFFER];
//     ssize_t rByte;
//     if (_client[cltIndx].getStatus() == CS_NEW) {
//         Request request;
//         _client[cltIndx].setRequest(request);
//     }
//     rByte = recv(_client[cltIndx].getFd(), buff, SRV_READ_BUFFER, 0);
//     if (rByte > 0)
//     {
//         /**
//          * Read request chunks-chunks, every chunk past to parseRequest,
//          * the parse of request must detect if the request is finished by setting ReqInfo to `CS_READING_DONE`
//          * else `CS_READING`
//          * */
//         // _client[cltIndx].getRequest().setBuffer(buffer);
//         _client[cltIndx]._reqInfo.buffer.insert(_client[cltIndx]._reqInfo.buffer.end(), buff, buff + rByte);
//         requestHandler(_client[cltIndx]);
//         if (_client[cltIndx]._reqInfo.reqStatus == CS_READING_DONE) {
//             _client[cltIndx]._reqInfo.buffer.clear();
//             _client[cltIndx]._sendInfo.resStatus = CS_START_SEND; /* To track first try of send-response */
//             return CS_READING_DONE;
//         }
//         return CS_READING;
//     }
//     else if (rByte == 0)
//     {
//         return CS_DISCONNECT;
//     }
//     else
//     {
//         std::cout << "Read Request: " << strerror(errno) << std::endl;
//         return CS_READING;
//     }
//     return CS_FATAL;
// }

void    Server::handleDisconnect(int index, std::vector<struct pollfd>& _pollfd) {
    std::cout << SERVER << RED << "User FD=" << _client[index ].getFd() << " Disconnected" << RESET << std::endl;
    if (_client[index]._cgiProc._readPipe != -1) {
        /* Close the CGI Pipe If that client request it and remove it from poll() */
        for (size_t i = _OpenPort + _client.size(); i < _pollfd.size(); i++) {
            if (_pollfd[i].fd == _client[index]._cgiProc._readPipe) {
                if (_client[index]._cgiProc._childPid != -1) {
                    _client[index]._cgiOut._code = 504;
                    CGI_errorResponse(_client[index], _client[index]._cgiOut._code);
                    std::cout << "Time-out Response .." << std::endl;
                    g_console.log(INFO, str("Child process killed with success."), BLUE);
                    std::cout << "PID:" << _client[index]._cgiProc._childPid << std::endl;
                    kill(_client[index]._cgiProc._childPid, SIGKILL);
                    int status;
                    waitpid(_client[index]._cgiProc._childPid, &status, 0);
                    /**
                     * TODO: response for that user.
                     */
                    _client[index]._cgiProc._childPid = -1;
                }
                close(_pollfd[i].fd);
                _pollfd.erase(_pollfd.begin() + i); /* remove pipe fd from pollfd{} */
                break;
            }
        }
    }
    if (_client[index]._sendInfo.fd != -1)
        close(_client[index]._sendInfo.fd);
    if (_client[index]._sendInfo.buff.size() != 0)
        _client[index]._sendInfo.buff.clear();
    if (_client[index]._uploadFd != -1) /* Close Upload Fd if exist */
        close(_client[index]._uploadFd);
    if (_client[index]._reqInfo.buffer.size() != 0)
        _client[index]._reqInfo.buffer.clear();
    close(_client[index].getFd());
    _pollfd.erase(_pollfd.begin() + _OpenPort + index); /* remove user fd from pollfd{} */
    _client.erase(_client.begin() + index); /* remove user fd from Client{} */
}

void    Server::closeClientConnection(void) {
    for (size_t i = 0; i < _client.size(); i++)
    {
        if (_client[i]._sendInfo.fd != -1)
            close(_client[i]._sendInfo.fd);
        if (_client[i]._uploadFd != -1)
            close(_client[i]._uploadFd);
        close(_client[i].getFd());
    }
    _client.clear();
}

Server::~Server() {
    std::cout << "<<<<< Server Obj distroyed >>>>>" << std::endl;
}

Client& Server::getClientReqCGI(int pipeFd) {
    /**
     * return Client that request the CGI-Script.
     */
    for (size_t i = 0; i < _client.size(); i++)
    {
        if (pipeFd == _client[i]._cgiProc._readPipe)
            return _client[i];
    }
    /**
     * in all cases it can't reach this line: return _client[0];
     * so, it's just to silent errors!*/
     g_console.log(WARNING, str("Can't find User That request for CGI"), BG_RED);
    return _client[0];
}

void    CGI_errorResponse(Client& client, int statusCode) {
    Request&    req = client.getRequest();
    std::cout << "Code : " << statusCode << std::endl;
    // Response    res = client.getResponse();
    ServerEntry* _srvEntry = getSrvBlock( client._serverBlockHint, req );
    
    getSrvErrorPage(client.getResponse(), _srvEntry, statusCode);
    str buffer = client.getResponse().generate();
    send(client.getFd(), buffer.c_str(), buffer.size(), 0);
    std::cout <<  "CGI error response: " << statusCode << std::endl;
}
