/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/08 11:46:46 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/08 15:52:27 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "TypeDefs.hpp"
#include "Response.hpp"
// #include <iostream>
#include <poll.h>
#include <unistd.h>
// #include <signal.h>

struct CGIOutput
{
	int		_code;
	str		_output;

	CGIOutput( void ) : _code(200) {}

	CGIOutput( int code, str output )
		: _code(code)
		, _output(output) {}
};

CGIOutput	cgiHandle( str& path, str& ntrp, str& reqBody );