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

void    SocketManager::acceptIncomingConn(void) {
    int 	totalEvent;
    size_t  count;
    int 	clientFd;

    struct sockaddr_in  *temp1 = (struct sockaddr_in*)_listenSocks[0].second;
    std::cout << "addres0-> " << temp1->sin_addr.s_addr << "port: " << ntohs(temp1->sin_port) << std::endl;
    struct sockaddr_in  *temp2 = (struct sockaddr_in*)_listenSocks[1].second;
    std::cout << "addres1-> " << temp2->sin_addr.s_addr << std::endl;
    struct sockaddr_in  *temp3 = (struct sockaddr_in*)_listenSocks[2].second;
    std::cout << "addres2-> " << temp3->sin_addr.s_addr << std::endl;

    std::vector<struct pollfd>  _pollfd(_listenSocks.size());
    std::cout << _pollfd.size() << std::endl;
    //set all listen socket to accept POLL_IN events
    for (size_t i = 0; i < _listenSocks.size(); i++)
    {
        _pollfd[i].fd = _listenSocks[i].first; // fill "struct pollfd" with listen socket FDs
        _pollfd[i].events = POLL_IN;
    }
    while (true)
    {
        if ((totalEvent = poll(_pollfd.data(), _pollfd.size(), -1)) == -1)
        {
            closeSockets(_listenSocks);
            throw   std::runtime_error(strerror(errno));
        }
        std::cout << totalEvent << " <<<<<<< event occured ! >>>>>>>" << std::endl;
		// check listen sockets for incomming Clients
        count = _listenSocks.size();
        for (size_t	i = 0; i < _listenSocks.size(); i++) {
            if (_pollfd[i].revents == POLL_IN)
            {
                if ((clientFd = accept(_pollfd[i].fd, NULL, NULL)) == -1)
                {
                    closeSockets(_listenSocks);
                    throw std::runtime_error(strerror(errno));
                }
                
                // addClients(clientFd, &clientAddr);
                std::cout << "after client added?" << std::endl;
			}
    	}
	}
}

std::vector<ServerEntry>&	SocketManager::retrieveServerBlock(size_t index) {
	struct sockaddr_in	*addr = (struct sockaddr_in*)_listenSocks[index].second;
    (void)addr;
	// int	port = ntohl(addr->sin_port);
	// int	address = addr->sin_addr.s_addr;
    return _config->_servers;
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