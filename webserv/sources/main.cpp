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

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 4096

std::string response =  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 28\r\n\r\n<h1>Hello from Webserv!</h1>";

// void	processRequest( const int& serverFD )
// {
// 	struct sockaddr_in clientAddr;
// 	socklen_t clientLen = sizeof(clientAddr);
// 	int clientFD = accept(serverFD, (sockaddr *)&clientAddr, &clientLen);

// 	if (clientFD >= 0) {
// 		int flag = fcntl(clientFD, F_GETFL, 0);
// 		fcntl( clientFD, F_SETFL, flag | O_NONBLOCK);

// 		clients.push_back(clientFD);
// 		std::cout << "\n__________________________________" << std::endl;
// 		std::cout << "New client [" << clientFD << "] connected" << std::endl;
// 		std::cout << "__________________________________\n" << std::endl;
// 	} else {
// 		std::cerr << "Accept failed" << std::endl;
// 	}
// }

// void handleClient( int clientFD )
// {
// 	char buf[1024] = {0};
// 	int bytesRead = recv(clientFD, buf, sizeof(buf) - 1, 0);

// 	if (bytesRead > 0) {
// 		std::cout << "Client [" << clientFD << "] sent: " << buf << std::endl;
// 		std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 14\r\nConnection: keep-alive\r\n\r\nHello, World!\n";
// 		send(clientFD, response.c_str(), response.length(), 0);
// 	} else if (bytesRead == 0) {
// 		std::cout << "Client [" << clientFD << "] disconnected" << std::endl;
// 		close(clientFD);
// 		clients.erase(std::remove(clients.begin(), clients.end(), clientFD), clients.end());
// 	} else if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
// 		std::cerr << "Error reading from client [" << clientFD << "]" << std::endl;
// 		close(clientFD);
// 		clients.erase(std::remove(clients.begin(), clients.end(), clientFD), clients.end());
// 	}
// }

int	main( void )
{
	// sock Init
	int	serverFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFD < 0) {
		std::cerr << "Failed to oprn a socket" << std::endl; return(1);
	}
	std::cout << "Socket ID [" << serverFD << "] is opened" << std::endl;

	// reusable addr
	int optVal = 1;
	if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0) {
		std::cout << "Failed to set SO_REUSEADDR" << std::endl;
		close(serverFD); return (1);
	}
	std::cout << "SO_REUSEADDR enabled" << std::endl;

	// serv Conf
	struct sockaddr_in	servAdrr;
	std::memset(&servAdrr, 0, sizeof(servAdrr));
	servAdrr.sin_family = AF_INET;
	servAdrr.sin_addr.s_addr = INADDR_ANY;
	servAdrr.sin_port = htons(PORT);

	// bind
	if (bind(serverFD, (struct sockaddr *)&servAdrr, sizeof(servAdrr)) < 0) {
		std::cerr << "Failed to bind socket ID: " << serverFD << std::endl;
		close(serverFD); return (1);
	}
	std::cout << "Binding success socket ID: " << serverFD << std::endl;

	// listen
	if (listen(serverFD, 5) < 0) {
		std::cerr << "Failed to listen socket ID: " << serverFD << std::endl;
		close(serverFD); return (1);
	}
	std::cout << "Server listening on port: " << PORT << std::endl;

	// setting nb-I/O
	fcntl(serverFD, F_SETFL, fcntl(serverFD, F_GETFL) | O_NONBLOCK);
	std::cout << "Socket ID [" << serverFD << "] set to non-blocking mode" << std::endl;

	std::vector<pollfd>	fds;

	pollfd serverPollFD;
	serverPollFD.fd = serverFD;
	serverPollFD.events = POLLIN;
	serverPollFD.revents = 0;
	fds.push_back(serverPollFD);

	char buf[BUFFER_SIZE];

	while (true)
	{
		std::cout << "Waiting for activity. On PORT [" << PORT << "]" << std::endl;

		int activity = poll(fds.data(), fds.size(), 4000);
		
		if (activity < 0) {
			std::cerr << "poll() failed" << std::endl;
			break ;
		}

		for (size_t i = 0; i < fds.size(); ++i) {
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == serverFD) {
					// accept client
					int clientFD = accept(serverFD, NULL, NULL);
					if (clientFD >= 0) {
						fcntl(clientFD, F_SETFL, fcntl(clientFD, F_GETFL) | O_NONBLOCK);
						pollfd clientPollFD;
						clientPollFD.fd = clientFD;
						clientPollFD.events = POLLIN;
						clientPollFD.revents = 0;
						fds.push_back(clientPollFD);
						std::cout << "New client connected: FD [" << clientFD << "]" << std::endl;
					}
				} else {
					// receive data from client
					int clientFD = fds[i].fd;
					int bytes = recv(clientFD, buf, BUFFER_SIZE - 1, 0);
					if (bytes <= 0) {
						std::cout << "Client disconnected: FD [" << clientFD << "]" << std::endl;
						close(clientFD);
						fds.erase(fds.begin() + i);
						--i;
						continue ;
					}
					buf[bytes] = '\0';
					std::cout << "Received: " << buf << std::endl; 
					send(clientFD, response.c_str(), response.size(), 0);
				}
			}
		}
	}
	close (serverFD);
	return (0);
}
