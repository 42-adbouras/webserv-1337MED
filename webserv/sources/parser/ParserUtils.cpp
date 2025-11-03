/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 11:30:53 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/29 17:14:17 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Config.hpp" 
#include <string> 

bool	startsWith( const str& path, const str& start )
{
    return (path.size() >= start.size() && path.compare(0, start.size(), start) == 0);
}

bool	validatePort( str& portStr, int line, int col , const str& path )
{
	for (size_t i = 0; i < portStr.size(); ++i) {
		if (!std::isdigit(portStr[i]))
			throw ParsingError("invalide port value " + portStr, path, line, col);
	}
	int port = std::atoi(portStr.c_str());
	if (port < PORT_MIN || port > PORT_MAX)
		throw ParsingError("port out of range " + portStr, path, line, col);
	return (true);
}

bool	isNum( const str& s )
{
	if (s.empty())
		return (false);
	for (size_t i = 0; i < s.size(); ++i) {
		if (!std::isdigit((static_cast<unsigned char>(s[i]))))
			return (false);
	} return (true);
}

bool	validHost( str& host )
{
	if (host.empty()) return (false);
	if (host == "0.0.0.0" || host == "localhost")
		return (true);

	size_t	i = 0, size = host.size();
	int		octet = 0;

	while (i < size) {
		int val = 0;
		int digits = 0;

		while (i < size && std::isdigit(host[i])) {
			val = val * 10 + (host[i] - '0');
			if (++digits > 3)
				return (false);
			++i;
		}
		if (digits == 0 || val > 255)
			return (false);

		++octet;
		if (octet == 4)
			return (i == size);
		if (i >= size || host[i] != '.')
			return (false);
		++i;
	}
	return (false);
}