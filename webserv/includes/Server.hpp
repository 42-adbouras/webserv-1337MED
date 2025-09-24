#pragma once

// #include "Request.hpp"
// #include "Response.hpp"
// #include "Utils.hpp" // IWYU pragma: keep
// #include <iostream> // IWYU pragma: keep
// #include <map>
// #include <vector>
// #include <poll.h>
// #include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "Config.hpp"
#include "ServerExcept.hpp"
#include <fcntl.h>
// typedef Data;
// #include <cstring>

// #define BUFFER_SIZE 4096

// typedef std::string str;
struct Data;

class Server
{
	private:
		std::vector<int>	_pollFd;
		Data				*_data;
	public:
		Server(Data data);
		static int	setNonBlocking(int sockFd);
		void		initListenSocket(void);
		~Server();
};
