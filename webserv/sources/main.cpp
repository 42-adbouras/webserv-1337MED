/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:36 by adbouras          #+#    #+#             */
/*   Updated: 2025/07/11 15:21:33 by adbouras         ###   ########.fr       */
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
	std::cout << "Socket ID: " << sockFD << " is opened" << std::endl;

	// serv Conf
	struct sockaddr_in	servAdrr;
	servAdrr.sin_family = AF_INET;
	servAdrr.sin_addr.s_addr = INADDR_ANY;
	servAdrr.sin_port = htons(PORT);

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
	// connect(sockFD, &servAdrr, sizeof(servAdrr));
	close (sockFD);
	return (0);
}
