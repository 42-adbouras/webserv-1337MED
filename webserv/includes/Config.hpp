/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 16:44:54 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/24 12:12:59 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Lexer.hpp"
#include "Server.hpp"
#include "TypeDefs.hpp"
#include <cstddef>
#include <map>
#include <set>
#include <exception>

struct CGIEntry
{
	str		_extention;
	str		_interpreter;
};

struct Location
{
	str					_path;
	std::vector<str>	_index;
	std::map<int, str>	_errorPages;
	size_t				_maxBodySize;
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
<<<<<<< HEAD
	str					_listen;
	str				_serverName;
	str					_root;
	str					_index;
	std::map<int, str>	_errorPages;
	int					_maxBodySize;
	str					_uploadStore;
	std::set<str>		_allowedMethosd;
	bool				_autoIndexSet;
	bool				_autoIndex;
	CGIEntry			_cgi;
=======
	std::vector< std::pair<str, int> >	_listen;
	str						_serverName;
	str						_root;
	std::vector<str>		_index;
	std::map<int, str>		_errorPages;
	size_t					_maxBodySize;
	str						_uploadStore;
	bool					_autoIndexSet;
	bool					_autoIndex;
	CGIEntry				_cgi;
>>>>>>> adbouras
	std::vector<Location>	_locations;
};

class ParsingError : public std::exception
{
private:
	str		_msg;
	int		_line;
	int		_col;
	str		_what;

public:
	ParsingError( const str& msg, int line, int col );
	virtual ~ParsingError() throw();
	const char*	what() const throw();
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
	const Token&	current( void ) const;
	bool			accept( TokenType t );
	bool			expect( TokenType t, const str& err );

	ServerEntry		parseServerBlock( void );
	Location		parseLocationBlock ( void );

	void			parseServerDir( ServerEntry& serv );
	void			parseLocationDir (Location& loc );

	void			fetchListen( ServerEntry& serv );
	// void			fetchPort( ServerEntry& serv );
	void			fetchServerName( ServerEntry& serv );
	void			fetchPath( str& path );
	void			fetchPathList( std::vector<str>& list );
	bool			fetchAutoIndex( void );
	void			fetchBodySize( size_t& size );
	void			fetchErrorPages( std::map<int, str>& errors );
	void			fetchCGI( CGIEntry& cgi );
	void			fetchMethods( std::set<str>& methods );
	void			fetchRedirect( Location& loc );
};
