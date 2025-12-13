#include "../../includes/serverHeader/Server.hpp"
#include "../../includes/serverHeader/Client.hpp"
#include "../../includes/serverHeader/ServerUtils.hpp"
#include <csignal>
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
    std::stringstream   oss;
    bool state = false;

    for (size_t i = 0; i < _client.size(); i++)
    {
        if (std::time(NULL) - _client[i].getStartTime() >= _client[i].getTimeOut())
        {
            oss << "User With `fd=" << _client[i].getFd() << "` Hors-Ligne!";
            g_console.log(TIME_OUT, oss.str(), BG_RED);
            state = true;
            handleDisconnect(i, pollFd);
        }
    }
    if (!state)
        std::cout << BG_GREEN << "[ INFO ]" << GREEN << "All Clients en-ligne" << RESET << std::endl;
    return state;
}

wsrv_timer_t Server::wsrv_find_next_timeout(void) {
    wsrv_timer_t now = std::time(NULL);
    wsrv_timer_t lower;
    std::stringstream   oss;

    for (size_t i = 0; i < _client.size(); ++i) {
        wsrv_timer_t elapsed = (now - _client[i].getStartTime());
        wsrv_timer_t timeout = _client[i].getTimeOut();
        ssize_t remaining = timeout - elapsed;

        oss << "Remaining For User `" << i + 1 << "`: " << remaining << 's';
        g_console.log(TIME_OUT, oss.str(), WHITE);
        if (remaining <= 0)
            return 0; // already timed out
        _client[i].setRemainingTime(remaining);
        oss.clear();
        oss.str("");
    }
    lower = _client[0].getRemainingTime();
    for (size_t i = 1; i < _client.size(); i++)
    {
        if (_client[i].getRemainingTime() < lower)
            lower = _client[i].getRemainingTime();
    }
    oss << "Remaining Time for Waiting events : " << lower << 's';
    g_console.log(TIME_OUT, oss.str(), MAGENTA);
    return (lower);
}

void    Server::addClients(Client client, std::vector<struct pollfd> &_pollfd) {
    struct pollfd   temp;

    client.setStartTime(std::time(NULL));
    client.setTimeOut(CLIENT_HEADER_TIMEOUT);/* timeOut to wait for the first request */
    client.setClientState(CS_NEW);
    client._alreadyExec = false;
    client._cgiProc._readPipe = -1;
    client._cgiProc._childPid = -1;
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
    std::vector<char>   buffer(SRV_READ_BUFFER);
    ssize_t rByte;
    if (_client[cltIndx].getStatus() == CS_NEW) {
        _client[cltIndx].setRequest(Request());
    }
    Request req = _client[cltIndx].getRequest();
    std::cout << "Server: Read Request from User fd=" << _client[cltIndx].getFd() << std::endl;
    rByte = recv(_client[cltIndx].getFd(), buffer.data(), buffer.size(), 0);
    if (rByte > 0)
    {
        /**
         * Read request chunks-chunks, every chunk past to parseRequest,
         * the parse of request must detect if the request is finished by setting ReqInfo to `CS_READING_DONE`
         * else `CS_READING`
         * */
        _client[cltIndx].getRequest().setBuffer(buffer);
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

void    Server::handleDisconnect(int index, std::vector<struct pollfd>& _pollfd) {
    std::stringstream   oss;

    oss << "User With `fd=" << _client[index ].getFd() << "` Disconnected!";
    if (_client[index]._cgiProc._readPipe != -1) {
        /* Close the CGI Pipe If that client request it and remove it from poll() */
        for (size_t i = _OpenPort + _client.size(); i < _pollfd.size(); i++) {
            if (_pollfd[i].fd == _client[index]._cgiProc._readPipe) {
                if (_client[index]._cgiProc._childPid != -1) {
                    CGI_errorResponse(_client[index], 504);
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
    {
        close(_client[index]._sendInfo.fd);
        _client[index]._sendInfo.fd = -1;
    }
    if (_client[index]._sendInfo.buff.size() != 0)
        _client[index]._sendInfo.buff.clear();
    close(_client[index].getFd());
    _pollfd.erase(_pollfd.begin() + _OpenPort + index); /* remove user fd from pollfd{} */
    _client.erase(_client.begin() + index); /* remove user fd from Client{} */
    g_console.log(SERVER, oss.str(), RED);
}

void    Server::closeClientConnection(void) {
    for (size_t i = 0; i < _client.size(); i++)
    {
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
    Response    res = client.getResponse();
    ServerEntry* _srvEntry = getSrvBlock( client._serverBlockHint, req );
    
    getSrvErrorPage(res, _srvEntry, statusCode);
    str buffer = res.generate();
    send(client.getFd(), buffer.c_str(), buffer.size(), 0);
    std::cout <<  "CGI error response: " << statusCode << std::endl;
}
