/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/08 11:46:46 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/09 13:10:01 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "TypeDefs.hpp"
// #include "Response.hpp"
#include "Request.hpp"
#include <iostream>
#include <poll.h>
#include <sys/poll.h>
#include <map>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

struct CGIContext
{
	str					_path;
	str					_ntrp;
	str					_method;
	str					_query;
	str					_name;
	str					_serverName;
	str					_serverPort;
	str					_serverAddr;
	str					_contenType;
	str					_body;
	std::map<str, str>	_headers;
};

struct CGIOutput
{
	int					_code;
	str					_output;

	CGIOutput( void ) : _code(200) {}

	CGIOutput( int code, str output )
		: _code(code)
		, _output(output) {}
};

CGIOutput	cgiHandle( Request& req );