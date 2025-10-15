#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "../Config.hpp"
#include "ServerExcept.hpp"

#include <fcntl.h>

struct Data;

class Server
{
	private:
        std::vector<int>    _clientSocks;
	public:
		Server();
		void	addClients(int clientFd);
		~Server();
	


};
