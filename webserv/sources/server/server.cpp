// #include "SocketManager.hpp"
#include "../includes/serverHeader/Server.hpp"
#include "Client.hpp"

Server::Server(int portOpen) : _OpenPort(portOpen) {
    std::cout << BG_GREEN << "[INFO] Server started — config=config.conf" << RESET << std::endl;
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
            state = true;
            std::cout << BG_RED << "[ TIME-OUT ]" << BG_BLUE << " __ user fd=" << _client[i].getFd() << " hors-ligne __" RESET << std::endl;
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

    for (size_t i = 0; i < _client.size(); ++i) {
        std::cout << "=========  " << _client[i].getTimeOut() << "  ========== " << std::endl;
        wsrv_timer_t elapsed = (now - _client[i].getStartTime());
        wsrv_timer_t timeout = _client[i].getTimeOut();
        wsrv_timer_t remaining = timeout - elapsed;
        std::cout << BG_GREEN << std::endl;
        std::cout << CYAN << "REMAINING FOR USER " << i+1 << ": " << remaining << RESET << std::endl;
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
    std::cout << BG_BLUE << "[ TIME-OUT ] ——— remaining time-out => " << lower << "s" << RESET << std::endl;
    return (lower);
}

void    Server::addClients(Client client, std::vector<struct pollfd> &_pollfd) {
    struct pollfd   temp;

    client.setStartTime(std::time(NULL));
    client.setTimeOut(CLIENT_HEADER_TIMEOUT);
    client.setClientState(CS_NEW);
    temp.fd = client.getFd();
    temp.events = POLLIN;
    temp.revents = 0;
    _client.push_back(client);
    _pollfd.push_back(temp);
}

Status    Server::readClientRequest(std::vector<struct pollfd>& pollFd, size_t cltIndex, size_t& loopIndex) {
    std::cout <<  CYAN << "REQUEST FROM USER WITH FD=" << GREEN << pollFd[loopIndex].fd << RESET << std::endl;
    requestHandler(_client[cltIndex]);
    if (_client[cltIndex].getStatus() == CS_DISCONNECT)
    {
        std::cout << RED << "[ NOTICE ] — recv returned 0 (connection closed by client) fd= " << pollFd[loopIndex].fd << RESET << std::endl;
        handleDisconnect(loopIndex - cltIndex, pollFd);
        std::cout << CYAN << "[ INFO ] — port and user fd that still en-ligne:" << YELLOW << std::endl;
        for (size_t k = 0; k < pollFd.size(); k++)
        {
            std::cout << pollFd[k].fd << "-";
        }
        std::cout << RESET << std::endl;
        loopIndex--;
        return S_CONTINUE;
    } 
    else
        pollFd[loopIndex].events |= POLLOUT;
    return NON;
}

void    Server::responsePart(size_t cltIndex) {
    sendResponse(_client[cltIndex]);
    if (_client[cltIndex].getStatus() == CS_KEEPALIVE)
    {
        std::cout << BLUE << "[ CONNECTION ] —— TCP connection still open to another request/response for USER fd = " << _client[cltIndex].getFd() << RESET << std::endl;
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
    std::cout << GREEN << "[ INFO ] —— response for user " << _client[cltIndex].getFd() << " has been send with success!" << RESET << std::endl;  
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
    
    close(_client[index].getFd());
    std::cout << RED << "USER WITH FD=" << _client[index].getFd() << " DISCONNECTED!" << std::endl;
    _pollfd.erase(_pollfd.begin() + _OpenPort + index);
    _client.erase(_client.begin() + index);
    // std::cout << "existing sockets: " << std::endl;
    // for (size_t i = 0; i < _pollfd.size(); i++)
    // {
    //     std::cout << _pollfd[i].fd << " - " ;
    // }
    // std::cout << std::endl;
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