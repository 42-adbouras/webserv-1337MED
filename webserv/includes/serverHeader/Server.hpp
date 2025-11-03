#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cerrno>
#include "SocketManager.hpp"
#include "../Config.hpp"
#include "ServerExcept.hpp"
// #include  "Client.hpp"
#include <fcntl.h>
// enum	Status;
typedef	size_t	wsrv_timer_t;
class 	Client;
struct Data;
class Server
{
	private:
        std::vector<Client>    _client;
	public:
		int						_OpenPort;
		Server(int portOpen);
		std::vector<Client>&	getListOfClients(void);
		void	request(Client& _clt);
		void	response(Client& _clt);
		void	addClients(Client client, std::vector<struct pollfd> &_pollfd);
		wsrv_timer_t	wsrv_find_next_timeout(void);	
		bool			wsrv_timeout_closer(std::vector<struct pollfd>& pollFd);
		Status			readClientRequest(std::vector<struct pollfd>& pollFd, size_t cltIndex, size_t& loopIndex);
		// void			generateResponse()
		void	handleDisconnect(int index, std::vector<struct pollfd>& _pollfd);
		void	closeClientConnection(void);
		~Server();
	


};