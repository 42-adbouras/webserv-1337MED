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

void	displayHashTable(const std::vector<TableOfListen> &table) {
	for (size_t i = 0; i < table.size(); i++)
	{
		std::cout << YELLOW;
		std::cout << "TABLE " << i + 1 << ": ==> " << "[ FD=" << table[i]._fd << ", IP=" << table[i]._ip << ", PORT=" << table[i]._port << ", SERVER_NAME=" << table[i]._serverName << " ]" << std::endl;
		std::cout << GREEN << "          ========================        " << std::endl;
	}
	std::cout << RESET << std::endl;
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
		// making a hash-table for all IP:PORT
		// ----------------------------------------------------------
		
		std::vector<TableOfListen>	tableOfListen;
		SocketManager	socketManager(config, tableOfListen);
		socketManager.setTableOfListen(tableOfListen);

		// displayHashTable(tableOfListen);
		for (size_t i = 0; i < tableOfListen.size(); i++)
		{
			for (size_t k = i; k < tableOfListen.size(); k++)
			{
				if (i == k || tableOfListen[i]._interfaceState.alreadyBinded == true)
					continue ; /* prevent check the same table */
				if (tableOfListen[i] == tableOfListen[k])
				{
					tableOfListen[k]._interfaceState.alreadyBinded = true;
				}
			}
		}/*****************************/
		// for (size_t i = 0; i < tableOfListen.size(); i++)
		// {
		// 	if (tableOfListen[i]._interfaceState.alreadyBinded == false)
		// 	{
		// 		std::cout  << "IP:PORT that not binded: " << tableOfListen[i]._ip << ":" << tableOfListen[i]._port << std::endl;
		// 	}
		// 	else
		// 		std::cout  << "IP:PORT that already binded: " << tableOfListen[i]._ip << ":" << tableOfListen[i]._port << std::endl;
		// }
		/*****************************/

		socketManager.initSockets();
		displayHashTable(tableOfListen);
		socketManager.listenToPorts();
		std::cout << " ========= " << socketManager.portCounter() << " ================" << std::endl;
		// exit(0);
		signal(SIGINT, signalHandler);
		socketManager.runCoreLoop();
		tableOfListen.clear();
		config._servers.clear();
		tokens.clear();
		// Server	server(data);
//	-------------------------------------------------------------------
		
		// CGIContext	cgi;
		// cgi._path = "www/cgi-scripts/hello.py";
		// cgi._name = "hello.py"; cgi._ntrp = "/usr/bin/python3";
		// cgi._method = "POST"; cgi._serverName = "ait-server";
		// cgi._serverAddr = "0.0.0.0"; cgi._serverPort = "8080";
		// cgi._contenType = "text/palin";
		// cgi._query["name"] = "world"; cgi._query["lang"] = "en";
		// cgi._headers["Host"] = "0.0.0.0:8080";
		// cgi._headers["Content-Type"] = "text/plain";
		// cgi._headers["User-Agent"] = "webserv-dev/0.1";
		// cgi._headers["Accept"] = "*/*";
		// cgi._body = "<user>\n\t"
		// 			"<name>Jane Doe</name>\n\t"
		// 			"<email>jane.doe@example.com</email>\n\t"
		// 			"<age>25</age>\n"
		// 			"</user>";
		// std::cout << cgiHandle(cgi)._output << std::endl;
		

		
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


// nik ys ab re ab ceb sim imad