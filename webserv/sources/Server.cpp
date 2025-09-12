/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 15:29:32 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/11 17:14:45 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server( int port, const str& root )
	: _port(port)
	, _root(root)
	, _serverFD(-1) {}

Server::~Server( void ) {}

bool	Server::init( void )
{
	_serverFD = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFD < 0) {
		std::cerr << "Failed to open socket" << std::endl;
		return (false);
	}
	std::cout << "Socket ID [" << _serverFD << "] opened" << std::endl;

	int optVal = 1;
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0) {
		std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
		return (false);
	}
	std::cout << "SO_REUSEADDR enabled" << std::endl;

	sockaddr_in	addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	if (bind(_serverFD, (sockaddr*)&addr, sizeof(addr)) < 0) {
		std::cerr << "Failed to bind socket" << std::endl;
		return (false);
	}
	std::cout << "Binding success socket ID: " << _serverFD << std::endl;

	if (listen(_serverFD, SOMAXCONN) < 0) {
		std::cerr << "Failed to listen" << std::endl;
		return (false);
	}
	std::cout << "Server listening on port: " << _port << std::endl;

	setNonBlocking(_serverFD);

	pollfd	p;
	p.fd = _serverFD;
	p.events = POLLIN;
	p.revents = 0;
	_fds.push_back(p);

	return (true);
}

void	Server::run()
{
	while (true)
	{
		std::cout << "Waiting for activity. On PORT [" << _port << "] Number of clients: " << _fds.size() - 1 << std::endl;
		int	activity = poll(_fds.data(), _fds.size(), 4000);

		if (activity < 0) {
			std::cerr << "poll() failed" << std::endl;
			break;
		}

		for (size_t i = 0; i < _fds.size(); ++i) {
			if (!(_fds[i].revents & POLLIN))
				continue;
			if (_fds[i].fd == _serverFD)
				acceptClient();
			else
				handleClient(i);
		}
	}
}

void	Server::acceptClient()
{
	int cfd = accept(_serverFD, NULL, NULL);
	if (cfd < 0) return;
	setNonBlocking(cfd);
	pollfd p; p.fd = cfd; p.events = POLLIN; p.revents = 0; _fds.push_back(p);
	_clientBuf[cfd] = "";
	std::cout << "New client connected: FD [" << cfd << "]" << std::endl;
}

void	Server::handleClient( size_t index )
{
	int		cfd = _fds[index].fd;
	char	buf[BUFFER_SIZE];
	int		bytes = recv(cfd, buf, BUFFER_SIZE - 1, 0);

	if (bytes <= 0) {
		disconnectClient(index);
		return ; 
	}
	
	buf[bytes] = '\0';
	std::cout << ">>>>>>>>>> REQUEST <<<<<<<<<<" << std::endl;
	std::cout << buf << std::endl;
	std::cout << ">>>>>>>>>>>>>> <<<<<<<<<<<<<<" << std::endl;
       
	_clientBuf[cfd] += buf;

	if (_clientBuf[cfd].find("\r\n\r\n") != str::npos)
		processRequest(cfd);
}

void	Server::processRequest( int cfd )
{
    str&	raw = this->_clientBuf[cfd];
    Request	req;

    if (!req.parse(raw)) {
		this->_clientBuf[cfd].clear();
		return;
	}

    size_t q = req.getPath().find('?');
    if (q != str::npos)
		req.getPath() = req.getPath().substr(0, q);
	
    std::cout << "Request from client [" << cfd << "]: " << req.getMethod() << " " << req.getPath() << std::endl;

    Response	resp;
    if (req.getMethod() == "GET")
		resp = Response::fromFile(req.getPath(), this->_root.c_str());
    else
		resp = Response::methodNotAllowed();

    str	out = resp.toString();
	std::cout << ">>>>>>>>>> RESPONSE <<<<<<<<<<" << std::endl;
	std::cout << out << std::endl;
	std::cout << ">>>>>>>>>>>>>>  <<<<<<<<<<<<<<" << std::endl;
    send(cfd, out.c_str(), out.size(), 0);
    _clientBuf[cfd].clear();
}

void	Server::disconnectClient( size_t index )
{
	int	cfd = _fds[index].fd;

	std::cout << "Client disconnected: FD [" << cfd << "]" << std::endl;
	close(cfd);

	_clientBuf.erase(cfd);
	_fds.erase(_fds.begin() + index);
}
