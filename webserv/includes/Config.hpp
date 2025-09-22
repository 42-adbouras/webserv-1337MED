/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 16:44:54 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/21 18:46:01 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Lexer.hpp"
#include "Server.hpp"
#include "TypeDefs.hpp"
#include <cstddef>
#include <map>
#include <set>

struct CGIEntry
{
	str		_extention;
	str		_interpreter;
};

struct Location
{
	str					_path;
	std::map<int, str>	_errorPages;
	long				_maxBodySize;
	str					_uploadStore;
	std::set<str>		_allowedMethods;

	bool				_autoIndexSet;
	bool				_autoIndex;
	
	bool				_redirSet;
	int					_redirCode;
	str					_redirTarget;
	CGIEntry			_cgi;
};

struct ServerEntry
{
	str					_listen;
	stack_t				_serverName;
	str					_root;
	str					_index;
	std::map<int, str>	_errorPages;
	int					_maxBodySize;
	str					_uploadStore;
	std::set<str>		_allowedMethosd;
	bool				_autoIndexSet;
	bool				_autoIndex;
	CGIEntry			_cgi;
	std::vector<Location>	_locations;
};

class ParsingErrer
{
public:
	str		_msg;
	int		_col;
	int		_line;
	ParsingErrer( const str& msg, int line, int col );
};

struct Data
{
	std::vector<ServerEntry>	_servers;
};

class ConfigParser
{
private:
	TokensVector	_tokens;
	size_t			_index;

public:
	ConfigParser( const TokensVector& tokens );
	Data			parseTokens( void );

private:
	const Token&	current( void );
	bool			accept( TokenType t );
	bool			expect( TokenType t, const str& err );

	ServerEntry		parseServerBlock( void );
	Location		parseLocationBlock ( void );

	void			parseServerDir( ServerEntry& serv );
	void			parseLocationDir (Location& loc );

	void			parseListen( ServerEntry& serv );
	void			parsePort( ServerEntry& serv );
};