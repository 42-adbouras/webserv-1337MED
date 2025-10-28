#include "../includes/serverHeader/Server.hpp"

Server::Server(int portOpen) : _OpenPort(portOpen) {
    std::cout << "server start ..." << std::endl;
}

std::vector<Client>& Server::getListOfClients(void) {
    return _client;
}

void    Server::addClients(Client client, std::vector<struct pollfd> &_pollfd) {
    struct pollfd   temp;
    temp.fd = client.getFd();
    temp.events = POLLIN;
    temp.revents = 0;
    // client._serverBlockHint[0]
    client.setStatus(NON);
    _client.push_back(client);
    _pollfd.push_back(temp);
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
        _clt.setStatus(KEEP_ALIVE);
    }
    if (readByte == 0) 
    {
        _clt.setStatus(DISCONNECT);
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
}

// status    Server::statOfUser(int clFd) const {
//     return DISCONNECT
// }

Server::~Server() {
    std::cout << "<<<<< Server Obj distroyed >>>>>" << std::endl;
}