#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cerrno>
#include "../Config.hpp"
#include "ServerExcept.hpp"
#include  "Client.hpp"
#include <fcntl.h>

struct Data;

class Server
{
	private:
        std::vector<Client>    _client;
	public:
		Server();
		void	request(int clfd);
		void	response(int clfd);
		void	addClients(Client client, std::vector<struct pollfd> &_pollfd);
		// bool	statOfUser(int clFd) const;
		void	handleDisconnect(int index, std::vector<struct pollfd>& _pollfd);
		~Server();
	


};
