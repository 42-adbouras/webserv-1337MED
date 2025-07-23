/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:43 by adbouras          #+#    #+#             */
/*   Updated: 2025/07/21 15:39:04 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include <poll.h>

class Webserv
{
private:
	int					_sockFD;
	std::vector<int>	_clientFDs;

public:
	Webserv( void );
	Webserv( const Webserv& right );
	~Webserv( void );
	Webserv& operator=( const Webserv& right );

	void	setSocket( int sockFD );
	int		getSocket( void ) const;
};
