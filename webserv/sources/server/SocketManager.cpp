#include "SocketManager.hpp"
#include "ServerUtils.hpp"
#include "Utils.hpp"
SocketManager::SocketManager(Data& config) : _config(&config) {
    std::cout << "Start setuping server" << std::endl;
    // std::cout << "address: " << _config->_servers[0]._listen[0].first << std::endl;
}

int SocketManager::setNonBlocking(int fd) {
    int flag = fcntl(fd, F_GETFL);
    return (fcntl(fd, F_SETFL, flag | O_NONBLOCK));
}

void    SocketManager::initSockets(void) {
    struct  addrinfo    hints, *results;
    int status, optVal;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //tell getaddrinfo() that the returned address will be used
    //               for binding a server socket, not for connecting as a client.
    std::set<std::string>::iterator it;
    for (size_t i = 0; i < _config->_servers.size(); i++)
    {
        it = _config->_servers[i]._portStr.begin();
        while (it != _config->_servers[i]._portStr.end())
        {
            status = getaddrinfo(_config->_servers[i]._listen.c_str(), (*it).c_str(), &hints, &results);
            if (status != 0)
            {
                std::cerr << gai_strerror(errno) << std::endl;
                throw std::runtime_error("Exception");
            }
            int fd = socket(results->ai_family, results->ai_socktype, IPPROTO_TCP);
            if (fd == -1) {
                closeSockets(_listenSocks);
                throw ServerExcept(errno);
            }
            optVal = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) == -1)
            {
                closeSockets(_listenSocks);
                throw std::runtime_error(strerror(errno));
            }
            SocketManager::setNonBlocking(fd);
            _listenSocks.push_back(std::make_pair(fd, &(*reinterpret_cast<struct sockaddr*>(results->ai_addr))));
            bindSockets();
            freeaddrinfo(results);
            it++;
        }
    }
}

void    SocketManager::bindSockets(void) {
    int index = _listenSocks.size() - 1;
    int status = bind(_listenSocks[index].first, _listenSocks[index].second, sizeof(struct sockaddr));
    if (status != 0) {
        std::cout << "in the bind func " << std::endl;
        closeSockets(_listenSocks);
        throw std::runtime_error(strerror(errno));
    }
    std::cout << "socket " << index + 1 << ": binded successfull" << std::endl;
}

void    SocketManager::listenPorts(void) {
	for (size_t i = 0; i < _listenSocks.size(); i++)
    {
        if (listen(_listenSocks[i].first, SOMAXCONN) == -1)
        {
            closeSockets(_listenSocks);
            std::runtime_error(strerror(errno));
        }
        std::cout << "socket " << _listenSocks[i].first << " listen!" << std::endl;
    }
}

void    SocketManager::setListenEvent(std::vector<struct pollfd>& _pollfd) {
    for (size_t i = 0; i < _listenSocks.size(); i++)
    {
        _pollfd[i].fd = _listenSocks[i].first; // fill "struct pollfd" with listen socket FDs
        _pollfd[i].events = POLLIN;
    }
}

bool    SocketManager::checkForNewClients( std::vector<struct pollfd>& _pollfd, Server& _server ) {
    int 	clientFd;

    for (size_t	i = 0; i < _listenSocks.size(); i++) 
    {
        if (_pollfd[i].revents & POLLIN)
        {
            if ((clientFd = accept(_pollfd[i].fd, NULL, NULL)) == -1)
            {
                closeSockets(_listenSocks);
                throw std::runtime_error(strerror(errno));
            }
            SocketManager::setNonBlocking(clientFd);
            _server.addClients(Client(clientFd), _pollfd);
            std::cout << "<<< client added! >>>" << std::endl;
        }
    }
    return true;
}

void    SocketManager::runCoreLoop(void) {
    int 	totalEvent;
    size_t  clientIndex = _listenSocks.size();
    Server  _server(_listenSocks.size());
    std::vector<struct pollfd>  _pollfd(_listenSocks.size());
    //set all listen socket to accept POLLIN events
    setListenEvent(_pollfd);
    std::vector<Client>&    _clients = _server.getListOfClients(); 
    while (true)
    {
        if ((totalEvent = poll(_pollfd.data(), _pollfd.size(), -1)) == -1)
        {
            closeSockets(_listenSocks);
            throw   std::runtime_error(strerror(errno));
        }
        // std::cout << "-->" << totalEvent << " <<<<<<< event occured ! >>>>>>>" << std::endl;
		// check listen sockets for incomming Clients
        checkForNewClients(_pollfd, _server);
        // check requests from clients
        // NOTE: client fds start from index = _listenSocks.size
        for (size_t i = clientIndex; i < _pollfd.size(); i++) {
            std::cout << "in client check --> " << _pollfd[i].fd << std::endl;
            // std::cout << "total of clinets--> " << _pollfd.size() - clientIndex << std::endl;
            if ( _pollfd[i].revents & (POLLHUP | POLLERR | POLLNVAL) ) {
                std::cout << "handle disco index ==> " << i - clientIndex << std::endl;
                _server.handleDisconnect(i - clientIndex, _pollfd);
            }
            else {
                if ( _pollfd[i].revents & POLLIN )
                {
                    // here we go for parse http request.
                    std::cout << "request accepted from user " << _pollfd[i].fd << std::endl;
                    _server.request(_clients[i - clientIndex]);
                    if (_clients[i - clientIndex].getStatus() == DISCONNECT)
                    {
                        std::cout << "I get new Status" << std::endl;
                        _server.handleDisconnect(i - clientIndex, _pollfd);
                        std::cout << "sockets that exist " << std::endl;
                        for (size_t k = 0; k < _pollfd.size(); k++)
                        {
                            std::cout << _pollfd[k].fd << std::endl;
                        }
                        continue;
                    }
                    else
                        _pollfd[i].events |= POLLOUT;
                }
                if ( _pollfd[i].revents & POLLOUT )
                {
                    _server.response(_clients[_pollfd.size() - clientIndex - 1]);
                    std::cout << "response for user " << _pollfd[i].fd << " has been generated with success!" << std::endl;
                    _pollfd[i].events &= ~POLLOUT;
                }
            }
        }
	}
}

//------ utils ------
size_t SocketManager::portCounter(void) const {
    size_t count = 0;
    size_t i = 0;
    size_t j;
    while (i < _config->_servers.size())
    {
        j = 0;
        while (j < _config->_servers[i]._listen.size())
        {
            j++;
            count++;
        }
        i++;
    }
    return count;
}