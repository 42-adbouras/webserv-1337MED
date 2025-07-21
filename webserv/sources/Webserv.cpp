/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 15:29:32 by adbouras          #+#    #+#             */
/*   Updated: 2025/07/21 15:39:29 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

Webserv::Webserv( void ) : _sockFD(0) { }

Webserv::Webserv( const Webserv& right )
{
	*this = right;
}

Webserv::~Webserv( void ) { }

Webserv& Webserv::operator=( const Webserv& right )
{
	this->_sockFD = right.getSocket();
	return (*this);
}

void Webserv::setSocket( int sockFD )
{
	this->_sockFD = sockFD;
}

int Webserv::getSocket( void ) const
{
	return (this->_sockFD);
}