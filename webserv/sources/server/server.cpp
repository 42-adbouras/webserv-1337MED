#include "../includes/serverHeader/Server.hpp"

Server::Server() {
    std::cout << "server start ..." << std::endl;
}

void    Server::addClients(Client client, std::vector<struct pollfd> &_pollfd) {
    struct pollfd   temp;
    temp.fd = client.fd;
    temp.events = POLLIN | POLLOUT;
    _client.push_back(client);
    _pollfd.push_back(temp);
    std::cout << "client with fd: " << clientFd << " connected" << std::endl;
}

void    Server::response(int clfd) {
    ssize_t sendByte;
    const char  *data = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 48\r\n"
        "\r\n"
        "<html><body><h1>Hello from C Server!</h1></body></html>";
    if ((sendByte = send(clfd, data, std::strlen(data), 0)) == -1)
    {
        std::cerr << strerror(errno) << "-> can't send data to " << clfd << std::endl;
    }
}

void    Server::request(int clfd){
    ssize_t readByte;
    char    buffer[1024];

    if ((readByte = recv(clfd, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[readByte] = '\0';
        std::cout << "request ->\n" << buffer << std::endl;
    }
    if (readByte == 0) 
    {
        std::cout << "connection is closed by the user ->" << clfd << std::endl;
    }
    else if (readByte < 0)
    {
        std::cerr << "recv set errno to: " << strerror(errno) << std::endl;
    }
}

void    Server::handleDisconnect(int index, std::vector<struct pollfd>& _pollfd) {
    close(_pollfd[index].fd);
    _pollfd.erase(_pollfd.begin() + index);
    _clientSocks.erase(_clientSocks.begin() + 1);
    std::cout << "client fd " << _pollfd[index].fd << " disconnect" << std::endl;

}

// status    Server::statOfUser(int clFd) const {
//     return DISCONNECT
// }

Server::~Server() {}