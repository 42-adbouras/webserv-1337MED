#pragma once

// #include "Request.hpp"
// #include "Response.hpp"
// #include "Utils.hpp" // IWYU pragma: keep
// #include <iostream> // IWYU pragma: keep
// #include <map>
// #include <vector>
// #include <poll.h>
// #include <string>
// #include <sys/socket.h>
// #include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>
// #include <cstring>
// #include <fcntl.h>

// #define BUFFER_SIZE 4096

// typedef std::string str;

class Server
{
	private:
		std::vector<int>	pollFd;
		
	public:
		Server();
		~Server();
};
