/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 17:05:39 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/17 17:19:10 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

ParsingErrer::ParsingErrer( const str& msg, int line, int col )
	: _msg(msg)
	, _line(line)
	, _col(col) {}

ConfigParser::ConfigParser( const TokensVector& tokens )
	: _tokens(tokens) {}

const Token&	ConfigParser::current( void )
{
	return (this->_tokens[this->_index]);
}

bool			ConfigParser::accept( TokenType type )
{
	if (this->_tokens[this->_index]._type == type) {
		++(this->_index);
		return (true);
	}
	return (false);
}

bool			ConfigParser::expect( TokenType type, const str& err )
{
	if (accept(type))
		return (true);
	exit(1); // throw
}