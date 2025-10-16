#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <iostream>
#include <vector>
#include "../Config.hpp"
#include "ServerExcept.hpp"

#include <fcntl.h>

struct Data;

enum	status {
	DISCONNECT,
	ALIVE
};

class Server
{
	private:
        std::vector<int>    _clientSocks;
	public:
		Server();
		void	addClients(int clientFd, std::vector<struct pollfd> &_pollfd);
		// bool	statOfUser(int clFd) const;
		void	handleDisconnect(int index, std::vector<struct pollfd>& _pollfd);
		~Server();
	


};
