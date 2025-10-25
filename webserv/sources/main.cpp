/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:36 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/24 14:07:08 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/serverHeader/Server.hpp"
#include "../includes/Lexer.hpp"
#include "../includes/TypeDefs.hpp"
#include "../includes/Config.hpp"
#include "../includes/serverHeader/SocketManager.hpp"
#include <fstream>
#include <sstream>
#include <vector> // IWYU pragma: keep

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

bool	validFile( const str& path )
{
	if (path.size() < 5)
		return (false);
	const str	ext = path.substr(path.size() - 5);
	return (ext == ".conf");
}

str		readConfig( const str& path )
{
	if (path.empty())
		throw std::invalid_argument("[InvalidConfigPathException]");
	if (!validFile(path))
		throw std::invalid_argument("[InvalidConfigFileException]");

	std::ifstream	in(path.c_str());
	if (!in.is_open())
		throw std::runtime_error("[FailedToOpenFileException]");

	std::ostringstream	oss;
	oss << in.rdbuf();

	if (!in && in.eof())
		throw std::runtime_error("[ErrorReadingFileExeption]");

	return (oss.str());
}

int	main( int ac, char** av )
{
	if (ac < 2) {
		std::cerr << "Usage: ./webserv <config.conf>" << std::endl;
		return (1);
	} try {
		str				cfg = readConfig(av[1]);
		Lexer			lex(cfg);
		TokensVector	tokens = lex.tokenize();
		// printTokens(tokens);
		ConfigParser	p(tokens, av[1]);
		Data	config = p.parseTokens();
		// SocketManager	socketManager(config);
		// socketManager.initSockets();
		// socketManager.listenToPorts();
		// socketManager.runCoreLoop();
		// Server	server(data);

		
	} catch (std::exception& e) {
		// std::cerr << "Server " << std::endl;
		std::cerr << RED << e.what() << RESET << std::endl;
		return (1);
	}
	// std::ifstream	confStream(av[1]);
	// std::ostringstream out;
	// out << confStream.rdbuf();

	// Server	server(PORT, ROOT);

	// if (!server.init())
	// 	return (1);
	// server.run();
	return (0);
}
