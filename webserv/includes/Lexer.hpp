/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 18:30:03 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/16 19:12:13 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "TypeDefs.hpp"
#include <vector>

enum TokenType
{
	T_EOF,
	T_LBRACE,
	T_RBRACE,
	T_SEMI,
	T_STR,
	T_NUM,
	T_UNK
};

struct Token
{
	TokenType	_type;
	str			_token;
	int			_line;
	int			_col;
};


class Lexer
{
private:
	str			_src;
	int			_pos;
	int			_col;
	int			_line;

public:
	Lexer( const str& input );	
	TokensVector	tokenize( void );

private:
	bool	eof( void ) const;
	char	peek( void ) const;
	char	get( void );
	void	advance( void );
	Token	makeToken( TokenType type, const str& token, int line, int col );
	void	skip( void );
	Token	stringToken( void );
};
