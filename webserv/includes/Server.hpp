#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include "Utils.hpp"
#include <iostream>
#include <map>
#include <vector>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

#define BUFFER_SIZE 4096

typedef std::string str;

class Server
{
private:
	int						_port;
	str				 		_root;
	int						_serverFD;
	std::vector<pollfd>		_fds;
	std::map<int, str>		_clientBuf;

public:
	Server( int port, const str& root );
	~Server();

	bool	init( void );
	void	run( void );

private:
	void	acceptClient( void );
	void	handleClient( size_t index );
	void	processRequest( int cfd );
	void	disconnectClient( size_t index );
	str		methodNotAllowed( void );
};
