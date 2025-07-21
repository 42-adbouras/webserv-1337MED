/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:36 by adbouras          #+#    #+#             */
/*   Updated: 2025/07/21 15:41:16 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <cerrno>

#define PORT 8080

std::vector<int> clients;

void	processRequest( const int& sockFD )
{
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientFD = accept(sockFD, (sockaddr *)&clientAddr, &clientLen);

	if (clientFD >= 0) {
		int flag = fcntl(clientFD, F_GETFL, 0);
		fcntl( clientFD, F_SETFL, flag | O_NONBLOCK);

		clients.push_back(clientFD);
		std::cout << "\n__________________________________" << std::endl;
		std::cout << "New client [" << clientFD << "] connected" << std::endl;
		std::cout << "Total clients: " << clients.size() << std::endl;
		std::cout << "__________________________________\n" << std::endl;
	} else {
		std::cerr << "Accept failed" << std::endl;
	}
}

void handleClient( int clientFD )
{
	char buf[1024] = {0};
	int bytesRead = recv(clientFD, buf, sizeof(buf) - 1, 0);

	if (bytesRead > 0) {
		std::cout << "Client [" << clientFD << "] sent: " << buf << std::endl;
		std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 14\r\nConnection: keep-alive\r\n\r\nHello, World!\n";
		send(clientFD, response.c_str(), response.length(), 0);
	} else if (bytesRead == 0) {
		std::cout << "Client [" << clientFD << "] disconnected" << std::endl;
		close(clientFD);
		clients.erase(std::remove(clients.begin(), clients.end(), clientFD), clients.end());
		std::cout << "Total clients: " << clients.size() << std::endl;
	} else if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		std::cerr << "Error reading from client [" << clientFD << "]" << std::endl;
		close(clientFD);
		clients.erase(std::remove(clients.begin(), clients.end(), clientFD), clients.end());
	}
}

int	main( void )
{
	// sock Init
	int	sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFD < 0) {
		std::cerr << "Failed to oprn a socket" << std::endl; return(1);
	}
	std::cout << "Socket ID [" << sockFD << "] is opened" << std::endl;

	// setting nb-I/O
	int flag = fcntl(sockFD, F_GETFL, 0);
	if (flag == -1) {
		std::cout << "fcntl() Failed to get socket flags" << std::endl;
		close (sockFD); return (1);
	}
	if (fcntl(sockFD, F_SETFL, flag | O_NONBLOCK) == -1) {
		std::cout << "Failed to set socket non-blocking" << std::endl;
		close(sockFD); return (1);
	}
	std::cout << "Socket ID [" << sockFD << "] set to non-blocking mode" << std::endl;

	// serv Conf
	struct sockaddr_in	servAdrr;
	servAdrr.sin_family = AF_INET;
	servAdrr.sin_addr.s_addr = INADDR_ANY;
	servAdrr.sin_port = htons(PORT);

	int optVal = 1;
	if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0) {
		std::cout << "Failed to set SO_REUSEADDR" << std::endl;
		close(sockFD); return (1);
	}
	std::cout << "SO_REUSEADDR enabled" << std::endl;

	// bind
	if (bind(sockFD, (sockaddr *)&servAdrr, sizeof(servAdrr)) < 0) {
		std::cerr << "Failed to bind socket ID: " << sockFD << std::endl;
		close(sockFD); return (1);
	}
	std::cout << "Binding success socket ID: " << sockFD << std::endl;
	
	// listen
	if (listen(sockFD, 5) < 0) {
		std::cerr << "Failed to listen socket ID: " << sockFD << std::endl;
		close(sockFD); return (1);
	}
	std::cout << "Server listening on port: " << PORT << std::endl;

	fd_set	fr, fw, fe;
	struct timeval tv;
	
	while (true)
	{
		tv.tv_sec = 4; tv.tv_usec = 0;
		FD_ZERO(&fr); FD_ZERO(&fw); FD_ZERO(&fe);
		
		FD_SET(sockFD, &fr);
		int maxFD = sockFD;

		for (size_t i = 0; i < clients.size(); ++i) {
			FD_SET(clients[i], &fr);
			if (clients[i] > maxFD)
				maxFD = clients[i];
		}

		std::cout << "Waiting for activity on " << (clients.size() + 1) << std::endl;
		int activity = select(maxFD + 1, &fr, &fw, &fe, &tv);
		
		if (activity < 0) {
			std::cout << "select() failed" << std::endl;
			break ;
		} else if (activity == 0) {
			std::cout << "No activity on port: " << PORT << std::endl;
		} else {
			if (FD_ISSET(sockFD, &fr))
				processRequest(sockFD);

			for (size_t i = 0; i < clients.size(); ++i) {
				if (FD_ISSET(clients[i], &fr)) {
					handleClient( clients[i]);
				}
			}
		}
		// sleep(2);
	}
	for (size_t i = 0; i < clients.size(); ++i) {
		close(clients[i]);
	}
	close (sockFD);
	return (0);
}
