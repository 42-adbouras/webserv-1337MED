#include "SocketManager.hpp"
#include "ServerUtils.hpp"
#include "Utils.hpp"
#include "Client.hpp"
#include "CookiesSessionManager.hpp"

SocketManager::SocketManager(Data& config, std::vector<TableOfListen>& tableOfListen) : _config(&config), _tableOfListen(tableOfListen) {
    std::cout << BG_GREEN << "\n    [INFO] Start managing listening sockets...    \n" << RESET << std::endl;
}

void    SocketManager::setTableOfListen(std::vector<TableOfListen>& table) {
    std::set<std::pair<str, str> >::iterator    it;
    for (size_t serverId = 0; serverId < _config->_servers.size(); serverId++)
    {
        it = _config->_servers[serverId]._listen.begin();
        while (it != _config->_servers[serverId]._listen.end())
        {
            TableOfListen  tmp;
            tmp._fd = -1;
            tmp.addr = NULL;
            tmp._ip = it->first;
            tmp._port = it->second;
            tmp._serverName = _config->_servers[serverId]._serverName;
            tmp._serverBlockId = serverId;
            table.push_back(tmp);
            it++;
        }
    }
}

int SocketManager::setNonBlocking(int fd) {
    return (fcntl(fd, F_SETFL, O_NONBLOCK));
}

void    SocketManager::initSockets(void) {
    struct  addrinfo    hints, *results;
    int status;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //tell getaddrinfo() that the returned address will be used
    //               for binding a server socket, not for connecting as a client.
    for (size_t counter = 0; counter < _tableOfListen.size(); counter++)
    {
        status = getaddrinfo(_tableOfListen[counter]._ip.c_str(), _tableOfListen[counter]._port.c_str(), &hints, &results);
        if (status != 0)
            throw std::runtime_error(gai_strerror(errno));
        int fd = socket(results->ai_family, results->ai_socktype, IPPROTO_TCP);
        if (fd == -1) {
            closeListenSockets();
            throw ServerExcept(errno);
        }
        SocketManager::setNonBlocking(fd);
        _tableOfListen[counter]._fd = fd;
        _tableOfListen[counter].alreadyBinded = false;
        _tableOfListen[counter].addr = (reinterpret_cast<struct sockaddr*>(results->ai_addr));
        bindSockets(counter);
        freeaddrinfo(results);
    }

}

bool    SocketManager::checkIfAlreadyBinded(size_t index) const {
    for (size_t i = 0; i < index; i++)
    {
        if (_tableOfListen[i] == _tableOfListen[index])
            return true;
    }
    return false;
}

void    SocketManager::bindSockets(size_t counter) {
    int status = 0;
    int opValue = 1;

    if (!checkIfAlreadyBinded(counter) && (setsockopt(_tableOfListen[counter]._fd, SOL_SOCKET, SO_REUSEADDR, &opValue, sizeof(opValue)) != 0))
    {
        closeListenSockets();
        throw std::runtime_error(strerror(errno));
    }
    status = bind(_tableOfListen[counter]._fd, _tableOfListen[counter].addr, sizeof(struct sockaddr));
    if (status != 0 && errno == EADDRINUSE) {
        _tableOfListen[counter].alreadyBinded = true;
        close(_tableOfListen[counter]._fd);
        hanldVirtualHost(_tableOfListen[counter], counter);
        std::cout << "THIS: " << _tableOfListen[counter]._ip << ":" << _tableOfListen[counter]._port << ", already binded" << std::endl;
    }
    else  if (status == 0)
        std::cout << "socket " << _tableOfListen[counter]._fd << ": binded successfull" << std::endl;
}

void    SocketManager::hanldVirtualHost(TableOfListen& table, size_t index) {
    for (size_t i = 0; i < index; i++)
    {
        if (_tableOfListen[i] == table)
        {
            std::cout << "THE ACTUAL ONE: " << this->_tableOfListen[i]._ip << ":" << this->_tableOfListen[i]._port << std::endl;
            std::cout << RED << "IP:PORT that should be VIRTUAL => [ " << table._ip << ", " << table._port << " ] - [ " << _tableOfListen[i]._ip << ", " << _tableOfListen[i]._port << " ]" << RESET << std::endl;
            table._fd = _tableOfListen[i]._fd;
            break;
        }
    }
}
void    SocketManager::listenToPorts(void) {
    int status;
    errno = 0;
	for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        std::cout << GREEN << "<< " << _tableOfListen[i]._ip << ", " << _tableOfListen[i]._port << " >>" << RESET << std::endl;
        if (!_tableOfListen[i].alreadyBinded && (status = listen(_tableOfListen[i]._fd, SOMAXCONN)) == 0)
        {
            std::cout << "socket fd: " << _tableOfListen[i]._fd << " -> LISTENING!" << std::endl;
        }
        else if(status != 0)
        {
            closeListenSockets();
            std::runtime_error(strerror(errno));    
        }  
        if (errno == EADDRINUSE)
        {
            std::cout << " ==================== IN THE LISTEN FUNCTION ===================" << std::endl;
            std::cout << strerror(errno) << std::endl;
            closeListenSockets();
            throw std::runtime_error(strerror(errno));
        }
    }
}


void    SocketManager::setListenEvent(std::vector<struct pollfd>& _pollfd) {
    size_t  count = 0;
    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (!_tableOfListen[i].alreadyBinded)
        {
            _pollfd[count].fd = _tableOfListen[i]._fd; // fill "struct pollfd" with listen socket FDs
            _pollfd[count].events = POLLIN;
            count++;
        }
    }
}

bool    SocketManager::checkForNewClients( std::vector<struct pollfd>& _pollfd, Server& _server ) {
    int 	clientFd;
    for (size_t	i = 0; i < portCounter(); i++) 
    {
        if (_pollfd[i].revents & POLLIN)
        {
            std::cout << "CONNECTION COMMME FROM FD: " << _pollfd[i].fd << " PORT:"  << std::endl;
            if ((clientFd = accept(_pollfd[i].fd, NULL, NULL)) == -1)
            {
                std::cout << "IN ACCEPT FUNC" << std::endl;
                closeListenSockets();
                _server.closeClientConnection();
                throw std::runtime_error(strerror(errno));
            }
            _server.addClients(Client(clientFd, detectServerBlock(_pollfd[i].fd)), _pollfd);
            SocketManager::setNonBlocking(clientFd);
            std::cout << YELLOW << "<<< client added with fd: " << clientFd << " >>>" << RESET << std::endl;
        }
    }
    return true;
}

void    SocketManager::rmClientFromPoll(std::vector<struct pollfd>& _pollfd, size_t  cltSize) {
    for (size_t i = 0; i < cltSize; i++)
    {
        _pollfd.erase(_pollfd.begin() + portCounter() + i);
    }
}

Status  SocketManager::PollingForEvents(std::vector<struct pollfd>& pollFd, Server& server,size_t cltSize) {
    int 	        totalEvent;
    wsrv_timer_t    coreTimeOut;

    (cltSize == 0 ? coreTimeOut = -1 : coreTimeOut = server.wsrv_find_next_timeout()*1000);
    std::cout << BG_BLUE << "TIME-OUT = " << static_cast<int>(coreTimeOut) << RESET << std::endl;
    totalEvent = poll(pollFd.data(), pollFd.size(), static_cast<int>(coreTimeOut));
    if (totalEvent == 0 && cltSize > 0)
    {
        server.wsrv_timeout_closer(pollFd);
        std::cout << GREEN << "[ " << totalEvent << " ]" << RED<< " <<<<<<< EVENTS OCCURED ! >>>>>>>" << RESET << std::endl;
        return S_TIMEDOUT;
    }
    else if (totalEvent == -1)
    {
        closeListenSockets();
        server.closeClientConnection();
        throw   std::runtime_error(strerror(errno));
    }
    std::cout << GREEN << "[ " << totalEvent << " ]" << RED<< " <<<<<<< EVENTS OCCURED ! >>>>>>>" << RESET << std::endl;
    return NON;
}

void    SocketManager::runCoreLoop(void) {
    size_t                      clientStartIndex = portCounter();
    CookiesSessionManager       sessionManager;
    Server                      _server(sessionManager, portCounter());
    std::vector<struct pollfd>  _pollfd(portCounter());

    setListenEvent(_pollfd);    // set listen socket to wait for POLLIN events
    std::vector<Client>&    _clients = _server.getListOfClients(); 
    while (true)
    {
        if (PollingForEvents(_pollfd, _server, _clients.size()) == S_TIMEDOUT)
            continue;
        checkForNewClients(_pollfd, _server);   // check listen sockets if there is new connections
        for (size_t i = clientStartIndex; i < _pollfd.size(); i++) {    // check for request/response from clients
            if ( _pollfd[i].revents & (POLLHUP | POLLERR | POLLNVAL) ) {
                std::cout << RED << "[ INFO ] ——— connection closed: client=" << _pollfd[i].fd << " idx=" << (i - clientStartIndex) << RESET << std::endl;
                _server.handleDisconnect(i - clientStartIndex, _pollfd);
                i--;
            }
            else {
                if ( _pollfd[i].revents & POLLIN )
                {
                    if (_server.readClientRequest(_pollfd, i - clientStartIndex, i) == S_CONTINUE)
                        continue ;
                    }
                    if ( _pollfd[i].revents & POLLOUT )
                    // CGI handler
                    // if (_clients[i-clientStartIndex].getStatus() == CS_CGI_REQ) {
                    //     // before calling cgiHandle ,must check if already executed.
                    //     cgiHandle(); // @param req and _client[i-clientStartIndex].getBool() ==> alreadyExec.
                    //     // read from  pipe.

                    // }
                        // break;
                        
                {
                    _server.responsePart(i - clientStartIndex);
                    _pollfd[i].events &= ~POLLOUT;
                }
            }
        }
	}
}

serverBlockHint    SocketManager::detectServerBlock(int sockFd) const {
    serverBlockHint   tmp;
    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (sockFd == _tableOfListen[i]._fd)
        {
            tmp.push_back(std::make_pair(&_tableOfListen[i],  &(_config->_servers[_tableOfListen[i]._serverBlockId])));
            std::cout << BLUE << "HII THERE! I GET THE BLOCKE SERVER." << std::endl;
            std::cout << "IP=" << _tableOfListen[i]._ip << ", PORT=" << _tableOfListen[i]._port << YELLOW << ", DOMAINE-NAME=[" << _tableOfListen[i]._serverName << "]" << RESET << std::endl;
        }
    }
    return tmp;
}

void    SocketManager::closeListenSockets(void) const {
    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (!_tableOfListen[i].alreadyBinded)
            close(_tableOfListen[i]._fd);
    }
    std::cout << RED << "LISTENING SOCKETS CLOSED!" << RESET << std::endl;
}

//------ utils ------
size_t SocketManager::portCounter(void) const {
    size_t count = 0;
    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (!_tableOfListen[i].alreadyBinded)
            count++;
    }
    return count;
}