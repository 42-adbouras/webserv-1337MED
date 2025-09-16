/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:36 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/16 19:33:05 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Lexer.hpp"
#include "TypeDefs.hpp"
#include <vector>

#define PORT 8080
#define ROOT "www"

const char*	tokenTypeName( TokenType t )
{
	switch (t) {
	case T_EOF:		return ("T_EOF");
	case T_LBRACE:	return ("T_LBRACE");
	case T_RBRACE:	return ("T_RBRACE");
	case T_SEMI:	return ("T_SEMI");
	case T_STR:		return ("T_STR");
	default:		return ("T_UNK");
	}
}

void	printTokens( const TokensVector& tokens )
{
	for (std::size_t i = 0; i < tokens.size(); ++i) {
		const Token& t = tokens[i];
		std::cout << "[" << t._line << ":" << t._col << "] "
				  << tokenTypeName(t._type) << "  '"
				  << t._token << "'" << std::endl;
	}
}

int	main( int ac, char** av )
{
	(void) ac;
	std::ifstream	confStream(av[1]);
	std::ostringstream out;
	out << confStream.rdbuf();

	Lexer			lex(out.str());
	TokensVector tokens = lex.tokenize();
	printTokens(tokens);
	// Server	server(PORT, ROOT);

	// if (!server.init())
	// 	return (1);
	// server.run();
	return (0);
}
