/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 17:05:39 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/01 16:19:41 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Config.hpp"

ParsingError::ParsingError( const str& msg, int line, int col )
	: _msg(msg)
	, _line(line)
	, _col(col)
{
	std::ostringstream	oss;

	oss << "ConfigParser::[" << _line << ":" << _col << "]:" << _msg;
	_what = oss.str();
}

ParsingError::~ParsingError() throw() {}

const char*	ParsingError::what() const throw()
{
	return (_what.c_str());
}

ConfigParser::ConfigParser( const TokensVector& tokens )
	: _tokens(tokens), _index(0) {}

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
		parseLocationDir(loc); // TODO
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
		fetchListen(serv);
		expect(T_SEMI, "Expected ';' AfterHostValue");
	} else if (cur._token == "port") {
		fetchPortList(serv);
		expect(T_SEMI, "Expected ';' AfterPortList");
	} else if (cur._token == "server_name") {
		std::cout << "found Server nAme:" << std::endl; 
		fetchServerName(serv);
		expect(T_SEMI, "Expected ';' After server_name");
	} else if (cur._token == "root") {
		std::cout << "found ROOT: " << std::endl;
		fetchPath(serv._root);
		expect(T_SEMI, "Expected ';' root");
	} else if (cur._token == "index") {
		std::cout << "found INDEX: " << std::endl;
		fetchPathList(serv._index);
		expect(T_SEMI, "Expected ';' after index");
	} else if (cur._token == "autoindex") {
		std::cout << "found AUTOINDEX: " << std::endl;
		serv._autoIndex = fetchAutoIndex();
		serv._autoIndexSet = true;
		expect(T_SEMI, "Expected ';' after autoindex");
	} else if (cur._token == "client_max_body_size") {
		std::cout << "found MAXBODYSIZE: " << std::endl;
		fetchBodySize(serv._maxBodySize);
		expect(T_SEMI, "Expected ';' after client_max_body_size");
	} else if (cur._token == "error_page") {
		std::cout << "found ERROR_CODE: " << std::endl;
		fetchErrorPages(serv._errorPages);
		expect(T_SEMI, "Expected ';' after error_page");
	} else if (cur._token == "cgi") {
		std::cout << "found CGI: " << std::endl;
		fetchCGI(serv._cgi);
		expect(T_SEMI, "Expected ';' after cgi");
	} else {
		throw ParsingError("UnknownServerDirective [" + cur._token + "]", cur._line, cur._col);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void	ConfigParser::parseLocationDir ( Location& loc )
{
	(void) loc;
	Token cur = current();

	++_index;
	if (cur._token == "root") {
		std::cout << "found ROOT: " << std::endl;
		fetchPath(loc._path);
		expect(T_SEMI, "Expected ';' root");
	} else if (cur._token == "index") {
		std::cout << "found INDEX: " << std::endl; 
		fetchPathList(loc._index);
		expect(T_SEMI, "Expected ';' after index");
	} else if (cur._token == "error_page") {
		std::cout << "found ERROR_PAGE: " << std::endl;
		fetchErrorPages(loc._errorPages);
		expect(T_SEMI, "Expected ';' after error_page");
	} else if (cur._token == "client_max_body_size") {
		std::cout << "found MAX_BODY_SIZE: " << std::endl;
		fetchBodySize(loc._maxBodySize);
		expect(T_SEMI, "Expected ';' after client_max_body_size");
	} else if (cur._token == "upload_store") {
		std::cout << "found UPLOAD_STORE: " << std::endl; 
		fetchPath(loc._uploadStore);
		expect(T_SEMI, "Expected ';' after upload_store");
	} else if (cur._token == "allowed_methods") {
		std::cout << "found ALLOWED_METHODS: " << std::endl;
		fetchMethods(loc._allowedMethods);
		expect(T_SEMI, "Expected ';' allowed_methods");
	} else if (cur._token == "autoindex") {
		std::cout << "found AUTOINDEX: " << std::endl;
		loc._autoIndex = fetchAutoIndex();
		loc._autoIndexSet = true;
		expect(T_SEMI, "Expected ';' after autoindex");
	} else if (cur._token == "redirect") {
		std::cout << "found REDIRECT: " << std::endl;
		fetchRedirect(loc);
		expect(T_SEMI, "Expected ';' after redirect");
	} else if (cur._token == "cgi") {
		std::cout << "found CGI: " << std::endl;
		fetchCGI(loc._cgi);
		expect(T_SEMI, "Expected ';' after cgi");
	} else {
		throw ParsingError("UnknownLocationDirective [" + cur._token + "]", cur._line, cur._col);
	}
}


bool	validatePort( int port, int line, int col )
{
	if (port < PORT_MIN || port > PORT_MAX)
		throw ParsingError("PortOutOfRange", line, col);
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
	if (host == "0.0.0.0" || host == "localhost")
		return (true);

	if (host.empty())
		return (false);

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

void	ConfigParser::fetchListen( ServerEntry& serv )
{
	(void) serv;
	Token	cur = current();

	if (!accept(T_STR))
		throw ParsingError("ExpectedListenValue", cur._line, cur._col);

	if (!validHost(cur._token))
		throw ParsingError("InvalidListenValue", cur._line, cur._col);

	if (!serv._listenSet) {
		serv._listen = cur._token;
		serv._listenSet = true;
	} else {
		std::cout << "[WARNING]: ListenAlreadySetTo::" << serv._listen << std::endl;
	}
	// if (cur._type != T_STR && cur._type != T_NUM)
	// 	throw ParsingError("ExpectedListenValue", cur._line, cur._col);

	// str	val = cur._token;
	// ++_index;

	// size_t colon = val.find(':');
	// if (colon == str::npos) {
	// 	if (!isNum(val))
	// 		ParsingError("ListenMustBeNumericOr host:port", cur._line, cur._col);
	// 	int port = std::atoi(val.c_str());
	// 	validatePort(port, cur._line, cur._col);
	// 	serv._listen.push_back(std::make_pair(str("0.0.0.0"), port));
	// 	std::cout << "\t host: " << "0.0.0.0" << std::endl;
	// 	std::cout << "\t port: " << port << std::endl;
	// } else {
	// 	str host = val.substr(0, colon);
	// 	str port = val.substr(colon + 1);
	// 	if (!isNum(port))
	// 		throw ParsingError("ListenMustBeNumericOr host:port", cur._line, cur._col);
	// 	int portVal = std::atoi(port.c_str());
	// 	validatePort(portVal, cur._line, cur._col);
	// 	serv._listen.push_back(std::make_pair(host, portVal));
	// 	std::cout << "\t host: " << host << std::endl;
	// 	std::cout << "\t port: " << portVal << std::endl;
	// }
}

void	ConfigParser::fetchPortList( ServerEntry& serv )
{
	(void) serv;
	Token cur = current();
	while (cur._type == T_NUM) {
		std::cout << "Current: " << cur._token << std::endl;
		int port = std::atoi(cur._token.c_str());
		if (validatePort(port, cur._line, cur._col)) {
			serv._port.insert(port);
			serv._portStr.insert(cur._token);
		}
		++_index;
		cur = current();
	}
	std::cout << cur._token << std::endl;
}

void	ConfigParser::fetchServerName( ServerEntry& serv )
{
	
	Token	cur = current();

	if (!accept(T_STR))
		throw ParsingError("ExpectedNameIn server_name", cur._line, cur._col);
	std::cout << "\t " << cur._token << std::endl;
	serv._serverName = cur._token;
}

void	ConfigParser::fetchPath( str& path )
{
	Token	cur = current();
	
	if (!accept(T_STR))
		throw ParsingError("ExpectedPath", cur._line, cur._col);
	// ++_index;
	path = cur._token;
	std::cout << "\t " << path << std::endl;
	// sanitize later
}

void	ConfigParser::fetchPathList( std::vector<str>& list )
{
	while (current()._type != T_SEMI) {
		Token cur = current();
		str	path = cur._token;
		// sanitize later
		std::cout << "\t " << path << std::endl;
		list.push_back(path);
		++_index;
	}
}

bool	ConfigParser::fetchAutoIndex( void )
{
	Token	cur = current();
	bool	value;

	if (!accept(T_STR))
		throw ParsingError("ExpectedPath on/off After autoindix", current()._line, current()._col);
	if (cur._token == "on" || cur._token == "true")
		value = true;
	else if (cur._token == "off" || cur._token == "false")
		value = false;
	else
		throw ParsingError("InvalidBooleanUse <on/off/true/false>", cur._line, cur._col);
	std::cout << "\t " << cur._token << std::endl;
	return (value);
}

void	ConfigParser::fetchBodySize( size_t& bodySize )
{
	Token	cur = current();
	std::cout << "\t " << cur._token << std::endl;

	if (!accept(T_STR))
		throw ParsingError("ExpectedSize", cur._line, cur._col);

	str	size  = cur._token;
	size_t	i = 0;
	while (i < size.size() && std::isdigit(size[i]))
		++i;

	if (!i) throw ParsingError("InvalideSize", cur._line, cur._col);

	str	nums = size.substr(0, i);
	str	unit = size.substr(i);

	size_t num = std::atol(nums.c_str());
	if (unit.empty() || unit.size() != 1)
		throw ParsingError("InvalidSizeUnit", cur._line, cur._col);
	char target = std::tolower(unit[0]);
	unsigned long mult;
	if (target == 'k')		mult = M_KILO;
	else if (target == 'm')	mult = M_MEGA;
	else if (target == 'g') mult = M_GEGA;
	else
		throw ParsingError("InvalidSizeUnit", cur._line, cur._col);
	
	size_t maxSize = std::numeric_limits<size_t>::max();
	if (num > maxSize / mult)
		throw ParsingError("SizeOverflow", cur._line, cur._col);
	bodySize = num * mult;
	std::cout << "\t result: " << bodySize << std::endl;
}

void	ConfigParser::fetchErrorPages( std::map<int, str>& errors )
{
	Token	cur = current();

	if (!accept(T_NUM))
		throw ParsingError("ExpectedErrorNumber", cur._line, cur._col);

	str	strCode = cur._token;
	if (strCode.size() != 3)
		throw ParsingError("InvalidErrorNumber", cur._line, cur._col);

	cur = current();
	if (!accept(T_STR))
		throw ParsingError("ExpectedPath", cur._line, cur._col);

	int code = std::atoi(strCode.c_str());
	errors[code] = cur._token;
	std::cout << "\t " << code << " : " << cur._token << std::endl;
}

void	ConfigParser::fetchCGI( CGIEntry& cgi )
{
	Token	cur = current();
	if (!accept(T_STR))
		throw ParsingError("ExpectedExtention", cur._line, cur._col);
		
	str	ext = cur._token;
	if (ext.size() < 2 || ext[0] != '.')
		throw ParsingError("InvalidExtention", cur._line, cur._col);

	cur = current();
	if (!accept(T_STR))
		throw ParsingError("ExpectedInterpreter", cur._line, cur._col);
	str inter = cur._token;
	cgi._extention = ext;
	cgi._interpreter = inter;
	std::cout << "\t " << ext << " : " << inter << std::endl;
}

void	ConfigParser::fetchMethods( std::set<str>& methods )
{
	const char* allowed[] = {"GET", "POST", "DELETE", NULL};
	
	while (current()._type == T_STR) {
		str	method = current()._token;
		++_index;
		bool ok = false;
		for (int i = 0; allowed[i]; ++i)
			if (method == allowed[i]) {
				ok = true;
				break ;
			}
		if (!ok)
			throw ParsingError("InvalidMethods", current()._line, current()._col);
		methods.insert(method);
	}
	if (methods.empty()) throw ParsingError("ExpectedMethod", current()._line, current()._col);
	for (std::set<str>::const_iterator it = methods.begin(); it != methods.end(); ++it)
		std::cout << "\t " << *it << std::endl;
}

void	ConfigParser::fetchRedirect( Location& loc )
{
	Token	cur = current();

	if (!accept(T_NUM))
		throw ParsingError("ExpectedRedirectCode", cur._line, cur._col);
		
	if (cur._token.size() != 3 || cur._token[0] != '3')
		throw ParsingError("InvalideRedirectCode", cur._line, cur._col);

	std::cout << "\t code: " << cur._token << std::endl;
	loc._redirCode = std::atoi(cur._token.c_str());
	cur = current();
	if (!accept(T_STR))
		throw ParsingError("ExpectedRedirectPath", cur._line, cur._col);
	std::cout << "\t path: " << cur._token << std::endl;
	loc._redirTarget = cur._token;
	loc._redirSet = true;
}
