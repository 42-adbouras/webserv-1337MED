#include "../../includes/serverHeader/SocketManager.hpp"
#include "../../includes/serverHeader/ServerUtils.hpp"

int SocketManager::setNonBlocking(int fd) {
    return (fcntl(fd, F_SETFL, O_NONBLOCK));
}

void    SocketManager::bindSockets(size_t counter) {
    int status = 0;

    status = bind(_tableOfListen[counter]._fd, reinterpret_cast<struct sockaddr*>(&_tableOfListen[counter].addr), sizeof(struct sockaddr));
    if (status != 0) {
        std::cerr << _tableOfListen[counter]._ip << ":" << _tableOfListen[counter]._port << ", " << counter;
        throw std::runtime_error(strerror(errno));
    }
    // else  if (status == 0) {
    //     std::cout << GREEN << "IP:PORT->" << _tableOfListen[counter]._ip << ":" << _tableOfListen[counter]._port << RESET << std::endl;
    //     std::cout << "socket fd " << _tableOfListen[counter]._fd << ": binded successfull" << std::endl;
    // }
}

void    SocketManager::listenToPorts(void) {
    int status;

	for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (_tableOfListen[i]._interfaceState.alreadyBinded)
            continue;
        if ((status = listen(_tableOfListen[i]._fd, SOMAXCONN)) == 0)
        {
            std::cout << LISTEN << CYAN <<  "Socket FD=" << _tableOfListen[i]._fd << " listening" << RESET << std::endl;
        }
        else if(status != 0)
        {
            closeListenSockets();
            std::cout << _tableOfListen[i]._ip << _tableOfListen[i]._port << std::endl;
            std::runtime_error(strerror(errno));
        }
    }
}