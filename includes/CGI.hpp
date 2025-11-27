/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/08 11:46:46 by adbouras          #+#    #+#             */
/*   Updated: 2025/11/23 15:18:04 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "TypeDefs.hpp"
#include "request.hpp" // IWYU pragma: keep
#include <iostream> // IWYU pragma: keep
#include <poll.h>
#include <sys/poll.h>
#include <map> // IWYU pragma: keep
#include <sys/wait.h>
#include <unistd.h>

struct CGIContext
{
	str			_path;
	str			_name;
	// str			_ntrp;
	str			_body;
	str			_method;
	str			_serverName;
	// str			_serverPort;
	// str			_serverAddr;
	// str			_contenType;
	QueryMap	_query;
	HeadersMap	_headers;
};

struct CGIProc
{
	bool		_error;
	pid_t		_childPid;
	int			_readPipe;
	int			_statusCode;
	CGIProc( bool err, int code )
		: _error(err), _statusCode(code) {}
	CGIProc( pid_t child, int rPipe, int code )
		: _error(false), _childPid(child)
		, _readPipe(rPipe), _statusCode(code) {}
	CGIProc() {}
};

struct CGIStatus
{
	bool	_error;
	int		_statusCode;
	CGIStatus( bool err, int code )
		: _error(err), _statusCode(code) {}
};

struct CGIOutput
{
	int			_code;
	str			_output;

	CGIOutput( void ) : _code(200) {}

	CGIOutput( int code, str output )
		: _code(code)
		, _output(output) {}
};

CGIProc		cgiHandle( CGIContext req, bool *alreadyExec );
// CGIOutput	readChild( const CGIProc& proc );
CGIOutput	readChild( Client& client );
