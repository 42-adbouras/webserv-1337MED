/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 16:44:54 by adbouras          #+#    #+#             */
/*   Updated: 2025/12/24 13:22:27 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "TypeDefs.hpp"
#include <cstddef>
#include <map>
#include <set>
#include <exception>
#include <sstream>
#include <limits>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "Lexer.hpp"


class ParsingError : public std::exception
{
private:
	str		_msg;
	int		_line;
	int		_col;
	str		_path;
	str		_what;

public:
	ParsingError( const str& msg, const str& path, const Token& cur );
	virtual ~ParsingError( void ) throw();
	const char*	what() const throw();
};

struct Data
{
	std::vector<ServerEntry>	_servers;
};

class ConfigParser
{
private:
	TokensVector				_tokens;
	size_t						_index;
	str							_path;

public:
	ConfigParser( const TokensVector& tokens, const str& path );
	Data			parseTokens( void );

private:
	const Token&	current( void ) const;
	bool			accept( TokenType t );
	bool			expect( TokenType t, const str& err );

	ServerEntry		parseServerBlock( void );
	Location		parseLocationBlock ( void );

	void			parseServerDir( ServerEntry& serv );
	void			parseLocationDir (Location& loc );

	void			fetchListen( ServerEntry& serv );
	// void			fetchPortList( ServerEntry& serv, const str& path );
	void			fetchServerName( ServerEntry& serv );
	void			fetchPath( str& path );
	void			fetchPathList( std::vector<str>& list );
	bool			fetchAutoIndex( void );
	void			fetchBodySize( size_t& size );
	void			fetchErrorPages( std::map<int, str>& errors );
	void			fetchCGI( Location& loc );
	void			fetchMethods( std::set<str>& methods );
	void			fetchRedirect( Location& loc );
	void			fetchTimeout( ServerEntry& serv, const str& type );

	void			mapLocationsRoot( ServerEntry& serv );
	void			printWarning( const str& arg, int line, int col );
};

str		readConfig( const str& path );
bool	validFile( const str& path );

bool	startsWith( const str& path, const str& start );
bool	validatePort( str& portStr, const Token& cur, const str& path );
bool	isNum( const str& s );
bool	validHost( str& host );
