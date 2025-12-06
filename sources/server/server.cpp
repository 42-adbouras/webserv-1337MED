#include "../../includes/serverHeader/Server.hpp"
#include "../../includes/serverHeader/Client.hpp"
#include "../../includes/serverHeader/ServerUtils.hpp"
#include <cstddef>
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
        wsrv_timer_t remaining = timeout - elapsed;

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
    client.setTimeOut(CLIENT_HEADER_TIMEOUT);
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
    // size_t  insetPos = _OpenPort + (_client.size() - 1);
    // if (insetPos <= _pollfd.size())
    // {
        _pollfd.insert(_pollfd.begin() + _OpenPort + (_client.size() - 1), temp);
    // }
    // else
        // _pollfd.push_back(temp);
}


ClientState Server::readRequest(size_t cltIndx) {
    // char    buffer[SRV_READ_BUFFER];
    std::vector<char>   buffer(SRV_READ_BUFFER);
    ssize_t rByte;

    if (_client[cltIndx].getStatus() == CS_NEW)
        _client[cltIndx].setRequest(Request());
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
        // req.setBuffer(buffer);

        requestHandler(_client[cltIndx]);
        if (_client[cltIndx]._reqInfo.reqStatus == CS_READING_DONE) {
            _client[cltIndx]._sendInfo.resStatus = CS_START_SEND; /* To track first try of send-response */
            return CS_READING_DONE;
        }
        return CS_READING;
    }
    else if (rByte == 0)
    {
        _client[cltIndx].setClientState(CS_DISCONNECT);
        return CS_DISCONNECT;
    }
    else
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            _client[cltIndx].setClientState(CS_READING);
            return CS_READING;
        }
        std::cout << "Read Request: " << strerror(errno) << std::endl;
    }
    return CS_FATAL;
}

Status    Server::readClientRequest(std::vector<struct pollfd>& pollFd, size_t cltIndex, size_t& loopIndex) {
    std::stringstream   oss;
    // ????????????????????????????????????//
    // don't use it anymore
    oss << "User With `fd=" << pollFd[loopIndex].fd << "` sent s a request";
    g_console.log(REQUEST, oss.str(), CYAN);
    requestHandler(_client[cltIndex]);
    _client[cltIndex]._alreadyExec = false; // for cgi
    // if (_client[cltIndex].getStatus() == CS_DISCONNECT) {
    //     std::stringstream   oss;
    //     oss << "peer closed connection, `fd=" << pollFd[loopIndex].fd << "`!";
    //     g_console.log(NOTICE, oss.str(), RED);
    //     handleDisconnect(cltIndex, pollFd);
    //     oss.clear();
    //     oss.str("");
    //     return S_CONTINUE;
    // }
    // else if (_client[cltIndex].getStatus() != CS_READING) {
    //     pollFd[loopIndex].revents = 0;
    //     pollFd[loopIndex].events |= POLLOUT;
    // }
    return NON;
}
 
void    Server::responsePart(size_t cltIndex) {
    std::stringstream   oss;

    if (_client[cltIndex]._sendInfo.resStatus == CS_WRITING_DONE)
    {
        oss << "Response For User `" << _client[cltIndex].getFd() << "` has been sent successfully.";
        g_console.log(INFO, oss.str(), GREEN);
        return;
    }

    // std::cout << GREEN << "[ INFO ] —— response for user " << _client[cltIndex].getFd() << " has been send with success!" << RESET << std::endl;  
    if (_client[cltIndex].getStatus() == CS_KEEPALIVE)
    {
        oss.clear();
        oss.str("");
        oss << "TCP Connection Still Open To Another Request/Response For User `fd=" << _client[cltIndex].getFd() << "`!";
        g_console.log(NOTICE, oss.str(), MAGENTA);
        int opt = 1;
        if (setsockopt(_client[cltIndex].getFd(), SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) != 0)
        {
            // closeListenSockets();
            closeClientConnection();
            throw std::runtime_error(strerror(errno));
        }
        _client[cltIndex].setStartTime(std::time(NULL));
        _client[cltIndex].setTimeOut(KEEPALIVE_TIMEOUT);
    }
}


void    Server::handleDisconnect(int index, std::vector<struct pollfd>& _pollfd) {
    std::stringstream   oss;

    close(_client[index].getFd());
    oss << "User With `fd=" << _client[index ].getFd() << "` Disconnected!";
    if (_client[index]._cgiProc._readPipe != -1) {
        /* Close the CGI Pipe If that client request it and remove it from poll() */
        for (size_t i = _OpenPort + _client.size(); i < _pollfd.size(); i++) {
                if (_pollfd[i].fd == _client[index]._cgiProc._readPipe) {
                    close(_pollfd[i].fd);
                    _pollfd.erase(_pollfd.begin() + i);
                    break;
                }
            }
            // _client[index]._cgiProc._readPipe = -1;
    }
    _pollfd.erase(_pollfd.begin() + _OpenPort + index);
    _client.erase(_client.begin() + index);
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

ClientState Server::sendResponse(Client& client) {
    (void)client;
    /**
     * here 
     * must send response ----------
     * 
     * 
     */
    return CS_WRITING_DONE;
}