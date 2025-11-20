// #include "SocketManager.hpp"
#include "../includes/serverHeader/Server.hpp"
#include "Client.hpp"

// CONSOLE g_console;
Server::Server(CookiesSessionManager& sessionManager, int portOpen) : _sessionManager(sessionManager), _OpenPort(portOpen) {
    (void) _sessionManager;
    g_console.log(SERVER, str("Server started — config=config.conf ..."), BG_CYAN);
    // std::cout << BG_GREEN << "[INFO] Server started — config=config.conf" << RESET << std::endl;
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
    // _pollfd.push_back(temp);
/**
 *  My Pollfd Layout is [listen sockets] [clinets][Cgi Pipes];
 *  So here i add new clinet after [listen sockets + clinet.size()-1]. (in the middle) 
 */
    size_t  insetPos = _OpenPort + (_client.size() - 1);
    if (insetPos <= _pollfd.size())
    {
        _pollfd.insert(_pollfd.begin() + insetPos, temp);
    }
    else
        _pollfd.push_back(temp);
}
// niki ys abd red ceb abl | anor  

Status    Server::readClientRequest(std::vector<struct pollfd>& pollFd, size_t cltIndex, size_t& loopIndex) {
    std::stringstream   oss;

    oss << "User With `fd=" << pollFd[loopIndex].fd << "` sent s a request";
    g_console.log(REQUEST, oss.str(), CYAN);
    _client[cltIndex]._alreadyExec = false;
    requestHandler(_client[cltIndex]);
    if (_client[cltIndex].getStatus() == CS_DISCONNECT) {

        std::stringstream   oss;
        oss << "recv(): Return 0; Connection Closed By User `fd=" << pollFd[loopIndex].fd << "`!";
        g_console.log(NOTICE, oss.str(), RED);
        handleDisconnect(cltIndex, pollFd);
        // std::cout << BG_GREEN << "Lists Of Clients:\n";
        // for (size_t i = 0; i < _client.size(); i++)
        // {
        //     std::cout << WHITE << _client[i].getFd() ;
        // }
        // std::cout << RESET << std::endl;
        oss.clear();
        oss.str("");
        // oss <<  "Ports and User's That Still En-Linge: ";
        // g_console.log(INFO, oss.str(), CYAN);
        // std::cout << CYAN << "[ INFO ] — port and user fd that still en-ligne:" << YELLOW << std::endl;
        // for (size_t k = 0; k < pollFd.size(); k++)
        // {
        //     std::cout << pollFd[k].fd;
        //     if (k + 1 < pollFd.size())
        //         std::cout << '-';
        // }
        // std::cout << std::endl;
        // loopIndex--;
        return S_CONTINUE;
    }
    else
        pollFd[loopIndex].events |= POLLOUT;
    return NON;
}
 
void    Server::responsePart(size_t cltIndex) {
    std::stringstream   oss;
    sendResponse(_client[cltIndex]);
    oss << "Response For User `" << _client[cltIndex].getFd() << "` has been sent successfully."; 
    g_console.log(INFO, oss.str(), GREEN);
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

void    Server::response(Client& _clt) {
    std::ostringstream  oss;
    oss << _clt.getFd();
    std::string sec(oss.str() + " </h1></body></html>");
    std::string req("HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 65\r\n"
        "\r\n"
        "<html><body><h1>fuck you mn hna l rachidiya ");
    ssize_t sendByte;
    std::string    data(req + sec); 
    //     ;
    if ((sendByte = send(_clt.getFd(), data.c_str(), data.size(), 0)) == -1)
    {
        std::cerr << strerror(errno) << "-> can't send data to " << _clt.getFd() << std::endl;
    }
    std::cout << YELLOW << "sizeof data: " << data.size() << ", sendByte: " << sendByte << RESET <<  std::endl;

    std::cout << data << std::endl;
}

void    Server::request(Client& _clt){
    ssize_t readByte;
    char    buffer[1024];

    if ((readByte = recv(_clt.getFd(), buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[readByte] = '\0';
        std::cout << "request ->\n" << buffer << std::endl;
        _clt.setClientState(CS_KEEPALIVE);
    }
    if (readByte == 0) 
    {
        _clt.setClientState(CS_DISCONNECT);
        std::cout << "connection is closed by the user ->" << _clt.getFd() << std::endl;
    }
    else if (readByte < 0) {
        std::cerr << "recv set errno to: " << strerror(errno) << std::endl;
    }
}

void    Server::handleDisconnect(int index, std::vector<struct pollfd>& _pollfd) {
    std::stringstream   oss;

    oss << "User With `fd=" << _client[index ].getFd() << "` Disconnected!";
    close(_client[index].getFd());
    g_console.log(SERVER, oss.str(), RED);
    _pollfd.erase(_pollfd.begin() + _OpenPort + index);
    _client.erase(_client.begin() + index);
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