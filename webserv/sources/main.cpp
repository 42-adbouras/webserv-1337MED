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
#define ROOT "./www"

// std::string response =  "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 31\r\n\r\n<h1>Hello from Webserv!</h1>";

int	setNonBlocking( int fd )
{
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

str	intToString( size_t v )
{
	std::ostringstream oss;
	oss << v; return (oss.str());
}

str	getContentType( const str& path )
{
	if (path.find(".html") != str::npos) return ("text/html");
	if (path.find(".css") != std::string::npos) return ("text/css");
	if (path.find(".js") != std::string::npos) return ("application/javascript");
	if (path.find(".png") != std::string::npos) return ("image/png");
	if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) return ("image/jpeg");
	return ("text/plain");
}

str	serveFile( const str& path )
{
	std::ifstream	file(path.c_str());
	if (!file.is_open()) {
		str notFound = "<h1>404 Not Found</h1>";
		return ("HTTP/1.1 404 Not Found\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: " + intToString(notFound.size()) + "\r\n"
				"Connection: keep-alive\r\n\r\n" + notFound);
	}

	std::ostringstream	content;
	content << file.rdbuf();
	str	body = content.str();
	str	contentType = getContentType(path);

	std::ostringstream	response;
	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: " << contentType << "\r\n"
			 << "Content-Length: " << body.size() << "\r\n"
			 << "Connection: keep-alive\r\n\r\n"
			 << body;
	return (response.str());
}

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
	if (listen(serverFD, SOMAXCONN) < 0) {
		std::cerr << "Failed to listen socket ID: " << serverFD << std::endl;
		close(serverFD); return (1);
	}
	std::cout << "Server listening on port: " << PORT << std::endl;

	// setting nb-I/O
	setNonBlocking(serverFD);
	std::cout << "Socket ID [" << serverFD << "] set to non-blocking mode" << std::endl;

	std::vector<pollfd>	fds;
	std::map<int, str>	clientBuf;

	pollfd serverPollFD;
	serverPollFD.fd = serverFD;
	serverPollFD.events = POLLIN;
	serverPollFD.revents = 0;
	fds.push_back(serverPollFD);

	char buf[BUFFER_SIZE];

	while (true)
	{
		std::cout << "Waiting for activity. On PORT [" << PORT << "]. Number of clients: " << fds.size() - 1 << std::endl;

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
						setNonBlocking(clientFD);
						pollfd clientPollFD;
						clientPollFD.fd = clientFD;
						clientPollFD.events = POLLIN;
						clientPollFD.revents = 0;
						fds.push_back(clientPollFD);
						clientBuf[clientFD] = "";
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
						clientBuf.erase(clientFD);
						--i;
						continue ;
					}
					buf[bytes] = '\0';
					clientBuf[clientFD] += buf;

					if (clientBuf[clientFD].find("\r\n\r\n") != std::string::npos) {
						str& request = clientBuf[clientFD];
						std::istringstream reqStream(request);
						str method, path, version;
						reqStream >> method >> path >> version;

						size_t queryPos = path.find("?");
						if ( queryPos != str::npos)
							path = path.substr(0, queryPos);

						std::cout << "Request from client [" << clientFD << "]: " << method << " " << path << std::endl;

						str response;
						if (method == "GET") {
							str fullPath = str(ROOT) + path;
							std::cout << "Looking for file at: " << fullPath << std::endl;
							if (!fullPath.empty() && fullPath[fullPath.size() - 1] == '/')
								fullPath += "index.html";
							std::cout << "Final path: " << fullPath << std::endl;
							response = serveFile(fullPath);
							send(clientFD, response.c_str(), response.size(), 0);
						} else {
							str notAllowed ="<h1>405 Method Not Allowed</h1>";
							response = "HTTP/1.1 405 Method Not Allowed\r\n"
								"Content-Type: text/html\r\n"
								"Content-Length: " + intToString(notAllowed.size()) + "\r\n"
								"Connection: keep-alive\r\n\r\n" + notAllowed;
							send(clientFD, response.c_str(), response.size(), 0);
						}
						clientBuf[clientFD].clear();
					}
				}
			}
		}
	}
	close (serverFD);
	return (0);
}
