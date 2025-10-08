/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:36 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/08 12:22:01 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp" // IWYU pragma: keep
#include "../includes/Lexer.hpp"
#include "../includes/TypeDefs.hpp"
#include "../includes/Config.hpp"
#include "../includes/CGI.hpp"
#include <vector> // IWYU pragma: keep

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

	return ( oss.str());
}

static void printErrorPages(const std::map<int, str>& errors, std::ostream& os) {
	if (errors.empty()) { os << "none"; return; }
	std::map<int, str>::const_iterator it = errors.begin();
	for (; it != errors.end(); ++it) {
		if (it != errors.begin()) os << ", ";
		os << it->first << "->" << it->second;
	}
}

static void printStringList(const std::vector<str>& v, std::ostream& os) {
	if (v.empty()) { os << "none"; return; }
	for (size_t i = 0; i < v.size(); ++i) {
		if (i) os << ' ';
		os << v[i];
	}
}

static void printMethods(const std::set<str>& m, std::ostream& os) {
	if (m.empty()) { os << "none"; return; }
	for (std::set<str>::const_iterator it = m.begin(); it != m.end(); ++it) {
		if (it != m.begin()) os << ' ';
		os << *it;
	}
}

static void printLocation(const Location& loc, std::ostream& os, size_t idx) {
	os << "  - location[" << idx << "] path: " << loc._path << "\n";
	os << "	  index: ";		printStringList(loc._index, os); os << "\n";
	os << "	  autoindex: "	<< (loc._autoIndexSet ? (loc._autoIndex ? "on" : "off") : "(inherit)") << "\n";
	os << "	  upload_store: " << (loc._uploadStore.empty() ? "none" : loc._uploadStore) << "\n";
	os << "	  max_body: "	 << (loc._maxBodySize ? loc._maxBodySize : 0) << " bytes\n";
	os << "	  allowed: ";	 printMethods(loc._allowedMethods, os); os << "\n";
	os << "	  error_pages: "; printErrorPages(loc._errorPages, os); os << "\n";
	if (loc._redirSet) {
		os << "	  redirect: " << loc._redirCode << " -> " << loc._redirTarget << "\n";
	}
	if (!loc._cgi._extention.empty() || !loc._cgi._interpreter.empty()) {
		os << "	  cgi: " << loc._cgi._extention << " -> " << loc._cgi._interpreter << "\n";
	}
}

void printServerEntry(const ServerEntry& s, std::ostream& os)
{
	os << "\n============== ServerEntry ==============\n";
	os << "  listen: " << s._listen << (s._listenSet ? " (set)" : " (default)") << "\n";

	os << "  ports: ";
	if (s._port.empty()) os << "none";
	else {
		for (std::set<int>::const_iterator it = s._port.begin(); it != s._port.end(); ++it) {
			if (it != s._port.begin()) os << ' ';
			os << *it;
		}
	}
	os << "\n";

	os << "  server_name: " << (s._serverName.empty() ? "none" : s._serverName) << "\n";
	os << "  root: "		<< (s._root.empty() ? "." : s._root) << "\n";

	os << "  index: ";
	printStringList(s._index, os);
	os << "\n";

	os << "  client_max_body_size: " << s._maxBodySize << " bytes\n";

	os << "  error_pages: ";
	printErrorPages(s._errorPages, os);
	os << "\n";

	if (!s._cgi._extention.empty() || !s._cgi._interpreter.empty()) {
		os << "  cgi: " << s._cgi._extention << " -> " << s._cgi._interpreter << "\n";
	} else {
		os << "  cgi: none\n";
	}

	os << "  locations: " << s._locations.size() << "\n";
	for (size_t i = 0; i < s._locations.size(); ++i)
		printLocation(s._locations[i], os, i);
}

int	main( int ac, char** av )
{
	(void) ac;
	(void) av;
	// if (ac < 2) {
	// 	std::cerr << "Usage: ./webserv <config.conf>" << std::endl;
	// 	return (1);
	// } try {
	// 	str				cfg = readConfig(av[1]);
	// 	Lexer			lex(cfg);
	// 	TokensVector	tokens = lex.tokenize();
	// 	// printTokens(tokens);
	// 	ConfigParser	p(tokens);
	
	// 	Data data = p.parseTokens();
	// 	for (size_t i = 0; i < data._servers.size(); ++i) {
    // 		printServerEntry(data._servers[i], std::cout);
	// 	}
	// } catch (std::exception& e) {
	// 	std::cerr << e.what() << std::endl;
	// 	return (1);
	// }
	str path = "www/cgi/hello.py";
	str ntrp = "/usr/bin/python3";
	str body = "/usr/bin/python3";

	CGIOutput out = cgiHandle(path, ntrp, body);
	std::cout << "code: " << out._code << std::endl;
	std::cout << "============================" << std::endl;
	std::cout << out._output << std::endl;
	std::cout << "============================" << std::endl;
	return (0);
}
