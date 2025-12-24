/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:36 by adbouras          #+#    #+#             */
/*   Updated: 2025/11/07 15:27:33 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/serverHeader/Server.hpp"
#include "../includes/Lexer.hpp"
#include "../includes/TypeDefs.hpp"
#include "../includes/Config.hpp"
#include "../includes/serverHeader/SocketManager.hpp"
#include "ServerUtils.hpp"
#include <fstream>
#include <sstream>
#include <vector> // IWYU pragma: keep
#include "../includes/CGI.hpp"
CONSOLE g_console;

const char*	tokenTypeName( TokenType t )
{
	switch (t) {
	case T_EOF:		return ("T_EOF");
	case T_LBRACE:	return ("T_LBRACE");
	case T_RBRACE:	return ("T_RBRACE");
	case T_SEMI:	return ("T_SEMI");
	case T_STR:		return ("T_STR");
	case T_NUM:		return ("T_NUM");
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



void	leak() {
	system("leaks webserv");
}

int	main( int ac, char** av )
{
	// atexit(leak);
	if (ac < 2) {
		std::cerr << "Usage: ./webserv <config.conf>" << std::endl;
		return (1);
	} try {
		str				cfg = readConfig(av[1]);
		Lexer			lex(cfg);
		TokensVector	tokens = lex.tokenize();
		ConfigParser	p(tokens, av[1]);
		Data	config = p.parseTokens();
		
		std::vector<TableOfListen>	tableOfListen;
		SocketManager	socketManager( config, tableOfListen );
		// making a hash-table for all IP:PORT
		socketManager.setTableOfListen( tableOfListen );
		socketManager.hanldVirtualHost( tableOfListen );
		// displayHashTable(tableOfListen);
		socketManager.initSockets();
		displayHashTable(tableOfListen);
		socketManager.listenToPorts();
		signal(SIGINT, signalHandler);
		socketManager.runCoreLoop();
		
	} catch (std::exception& e) {
		std::cerr << RED << e.what() << RESET << std::endl;
		return (1);
	}
	return (0);
}
