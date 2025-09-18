/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 16:44:54 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/17 17:05:28 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Lexer.hpp"
#include "TypeDefs"
#include "TypeDefs.hpp"
#include <cstddef>

class ParsingErrer
{
public:
	str		_msg;
	int		_col;
	int		_line;
	ParsingErrer( const str& msg, int line, int col );
};

class ConfigParser
{
private:
	TokensVector	_tokens;
	size_t			_index;
public:
	ConfigParser( const TokensVector& tokens );
private:
	const Token&	current( void );
	bool			accept( TokenType t );
	bool			expect( TokenType t, const str& err );
};