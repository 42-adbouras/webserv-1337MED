/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:36 by adbouras          #+#    #+#             */
/*   Updated: 2025/07/18 19:21:33 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

#define PORT 9909

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
	if (bind(sockFD, (sockaddr *)&servAdrr, sizeof(sockaddr)) < 0) {
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
	int nMaxFd = sockFD;
	struct timeval tv;
	
	while (true)
	{
		tv.tv_sec = 4; tv.tv_usec = 0;
		FD_ZERO(&fr); FD_ZERO(&fw); FD_ZERO(&fe);
		FD_SET(sockFD, &fr);
	
		std::cout << "Waiting for connections..." << std::endl;
		int activity = select(nMaxFd + 1, &fr, &fw, &fe, &tv);
		
		if (activity < 0) {
			std::cout << "select() failed" << std::endl;
			return (1);
		} else if (activity == 0) {
			std::cout << "No activity on port: " << PORT << std::endl;
		} else {
			std::cout << "Activity detected on [" << activity << "] fd" << std::endl;
		}
		sleep(2);
	}
	close (sockFD);
	return (0);
}
