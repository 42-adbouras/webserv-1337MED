/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 18:29:56 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/16 19:24:34 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"

Lexer::Lexer( const str& input )
	: _src(input)
	, _pos(0)
	, _col(1)
	, _line(1) {}

bool	Lexer::eof( void ) const
{
	return (this->_pos >= (int)this->_src.size());
}

char	Lexer::peek( void ) const
{
	return (eof() ? '\0' : this->_src[this->_pos]);
}

char	Lexer::get( void )
{
	char	c = peek();

	if (!eof()) {
		++(this->_pos);
		if (c == '\n') {
			++this->_line; this->_col = 1;
		} else {
			++(this->_col);
		}
	}
	return (c);
}

void	Lexer::advance( void )
{
	get();
}

Token	Lexer::makeToken( TokenType type, const str& token, int line, int col )
{
	Token	t;

	t._type  = type;
	t._token = token;
	t._line  = line;
	t._col   = col;
	return (t);
}

void	Lexer::skip( void )
{
	while (!eof())
	{
		char	c = peek();
		if (std::isspace(c)) {
			advance(); continue ;
		} if (c == '#') {
			while (!eof() && peek() != '\n')
				advance();
			continue ;
		}
		break ;
	}
}

TokensVector	Lexer::tokenize( void )
{
	TokensVector	tokens;

	while (true)
	{
		skip();
		if (eof()) {
			tokens.push_back(makeToken(T_EOF, "", this->_line, this->_col));
			break ;
		}
		char	c = peek();
		if (c == '{') {
			tokens.push_back(makeToken(T_LBRACE, "{", this->_line, this->_col));
			advance();
		} else if (c == '}') {
			tokens.push_back(makeToken(T_RBRACE, "}", this->_line, this->_col));
			advance();
		} else if (c == ';') {
			tokens.push_back(makeToken(T_SEMI, ";", this->_line, this->_col));
			advance();
		} else {
			tokens.push_back(stringToken());
			// advance();
		}
	}
	return (tokens);
}

Token	Lexer::stringToken( void )
{
	int	tLine= this->_line, tcol = this->_col;
	str	out;

	while (!eof())
	{
		char	c = peek();
		if (std::isspace(c) || c == '{' || c == '}' || c == ';' || c == '#')
			break ;
		out.push_back(get());
	}
	Token	token = makeToken(T_STR, out, tLine, tcol);
	return (token);
}