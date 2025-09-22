/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 17:05:39 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/22 17:08:37 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Config.hpp"


ParsingError::ParsingError( const str& msg, int line, int col )
: _msg(msg)
, _line(line)
, _col(col)
{
	std::ostringstream	oss;
	oss << "ConfigParser::" << _msg << " [" << _line << ":" << _col << "]";
	_what = oss.str();
}

ParsingError::~ParsingError() throw() {}

const char*	ParsingError::what() const throw()
{
	return (_what.c_str());
}

ConfigParser::ConfigParser( const TokensVector& tokens )
	: _tokens(tokens) {}

const Token&	ConfigParser::current( void ) const
{
	return (this->_tokens[this->_index]);
}

bool	ConfigParser::accept( TokenType type )
{
	if (this->_tokens[this->_index]._type == type) {
		++(this->_index);
		return (true);
	}
	return (false);
}

bool	ConfigParser::expect( TokenType type, const str& err )
{
	if (accept(type))
		return (true);
	throw ParsingError(err, current()._line, current()._col);
}

Data	ConfigParser::parseTokens( void )
{
	Data	data;
	// skip
	while(!accept(T_EOF)) {
		ServerEntry	s = parseServerBlock();
		data._servers.push_back(s);
		
		// skip
	}
	return (data);
}

ServerEntry	ConfigParser::parseServerBlock( void )
{
	Token	t = current();
	if (!(t._type == T_STR && t._token == "server"))
		throw ParsingError("Expected 'server'", t._line, t._col);
	++_index;
	if (!accept(T_LBRACE))
		throw ParsingError("Expected '{' after 'server'", current()._line, current()._col);

	ServerEntry	s;

	while (!accept(T_RBRACE)) {
		Token cur = current();
		if (cur._type == T_STR && cur._token == "location") {
			Location loc = parseLocationBlock(); // TODO
			s._locations.push_back(loc);
		} else if (cur._type == T_STR) {
			parseServerDir(s); // TODO
		} else {
			throw ParsingError("UnexpectedTokenInServerBody " + current()._token, current()._line, current()._col);
		}
	}
	return (s);
}

bool	startsWith( const str& path, const str& start )
{
    return (path.size() >= start.size() && path.compare(0, start.size(), start) == 0);
}

Location	ConfigParser::parseLocationBlock ( void )
{
	Token	token = current();
	++_index;
	Token	tPath = current();
	
	if (!(tPath._type == T_STR && startsWith(tPath._token, "/")))
		throw ParsingError("UnexpectedPathPrefix [" + tPath._token + "]", tPath._line, tPath._col);

	Location	loc;
	loc._path = tPath._token;
	++_index;
	if (!accept(T_LBRACE))
		throw ParsingError("Expected '{' AfterLocationPath", current()._line, current()._col);
	while (!accept(T_RBRACE)) {
		if (!(current()._type == T_STR))
			throw ParsingError("ExpectedLocationDirective", current()._line, current()._col);
		// parseLocationDir(loc); // TODO
	}
	return (loc);
}

void	ConfigParser::parseServerDir( ServerEntry& serv )
{
	(void) serv;
	Token	cur = current();
	++_index;

	if (cur._token == "listen") {
		std::cout << "found Listen" << std::endl;
		fetchListen(serv); // TODO
	} else if (cur._token == "port") {
		std::cout << "found Port" << std::endl; 
		// fetchPort(serv);
	} else if (cur._token == "server_name") {
		std::cout << "found Server nAme" << std::endl; 
		// fetchServerName(serv._serverName, cur._line, cur._col);
		expect(T_SEMI, "Expected ';' After server_name");
	} else {
		throw ParsingError("UnknownServerDirective [" + cur._token + "]", cur._line, cur._col);
	}
}

void	validatePort( int port, int line, int col )
{
	if (port < 1 || port > 65535)
		throw ParsingError("PortOutOfRange", line, col);
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


void	ConfigParser::fetchListen( ServerEntry& serv )
{
	(void) serv;
	Token	cur = current();
	std::cout << "current: " << cur._token << std::endl;

	if (cur._type != T_STR && cur._type != T_NUM)
		throw ParsingError("ExpectedListenValue", cur._line, cur._col);

	str	val = cur._token;
	++_index;

	size_t colon = val.find(':');
	if (colon == str::npos) {
		if (!isNum(val))
			ParsingError("ListenMustBeNumericOr host:port", cur._line, cur._col);
		int port = std::atoi(val.c_str());
		validatePort(port, cur._line, cur._col);
		serv._listen.push_back(std::make_pair(str(""), port));
	} else {
		str host = val.substr(0, colon);
		str port = val.substr(colon + 1);
		if (!isNum(port))
			throw ParsingError("ListenMustBeNumericOr host:port", cur._line, cur._col);
		int portVal = std::atoi(port.c_str());
		validatePort(portVal, cur._line, cur._col);
		serv._listen.push_back(std::make_pair(host, portVal));
	}
	expect(T_SEMI, "Expected ';' AfterListen");
}