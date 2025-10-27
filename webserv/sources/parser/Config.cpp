/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 17:05:39 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/25 20:11:07 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Config.hpp"

ParsingError::ParsingError( const str& msg, const str& path, int line, int col )
	: _msg(msg)
	, _line(line)
	, _col(col)
	, _path(path)
{
	std::ostringstream	oss;

	oss << "[ERROR]: configuration file " << _path << "["  \
		<< _line << ":" << _col << "]\n\t➜ " << _msg << ".";
	_what = oss.str();
}

ServerEntry::ServerEntry( void )
	: _listenSet(false)
{
	_listen.insert(std::make_pair("0.0.0.0", "8080"));
}

Location::Location( void )
	: _autoIndexSet(false)
	, _autoIndex(false)
	, _redirSet(false) {}

ParsingError::~ParsingError( void ) throw() {}

const char*	ParsingError::what() const throw()
{
	return (_what.c_str());
}

ConfigParser::ConfigParser( const TokensVector& tokens, const str& path )
	: _tokens(tokens), _index(0), _path(path) {}

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
	Token	cur = current();
	throw ParsingError(err, _path, cur._line, cur._col);
}

Data	ConfigParser::parseTokens( void )
{
	Data	data;

	while(!accept(T_EOF)) {
		ServerEntry	s = parseServerBlock();
		data._servers.push_back(s);
	}
	return (data);
}

ServerEntry	ConfigParser::parseServerBlock( void )
{
	Token	t = current();

	if (!(t._type == T_STR && t._token == "server"))
		throw ParsingError(EXP_SERV_DIR, _path, t._line, t._col);

	++_index;

	if (!accept(T_LBRACE))
		throw ParsingError(SERV_DIR_ERR, _path, current()._line, current()._col);

	ServerEntry	s;

	while (!accept(T_RBRACE)) {
		Token cur = current();

		if (cur._type == T_STR && cur._token == "location") {
			Location loc = parseLocationBlock();
			s._locations.push_back(loc);
		} else if (cur._type == T_STR) {
			parseServerDir(s);
		} else {
			throw ParsingError("unknown directive \"" + cur._token + "\"", _path, cur._line, cur._col);
		}
	}
	return (s);
}

Location	ConfigParser::parseLocationBlock ( void )
{
	Token	token = current();
	++_index;
	Token	tPath = current();
	
	if (!(tPath._type == T_STR && startsWith(tPath._token, "/")))
		throw ParsingError("UnexpectedPathPrefix [" + tPath._token + "]", _path, tPath._line, tPath._col);

	Location	loc;
	loc._path = tPath._token;
	++_index;
	if (!accept(T_LBRACE))
		throw ParsingError("expecting \"{\" after location directive", _path, current()._line, current()._col);
	while (!accept(T_RBRACE)) {
		if (!(current()._type == T_STR))
			throw ParsingError("expecting \"location\" directive", _path, current()._line, current()._col);
		parseLocationDir(loc);
	}
	return (loc);
}

void	ConfigParser::parseServerDir( ServerEntry& serv )
{
	(void) serv;
	Token	cur = current();
	++_index;

	if (cur._token == "listen") {
		fetchListen(serv);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "server_name") {
		fetchServerName(serv);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "root") {
		fetchPath(serv._root);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "index") {
		fetchPathList(serv._index);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} /*else if (cur._token == "autoindex") {
		serv._autoIndex = fetchAutoIndex();
		serv._autoIndexSet = true;
		expect(T_SEMI, "expected ';' after autoindex");
	} */
	else if (cur._token == "client_max_body_size") {
		fetchBodySize(serv._maxBodySize);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "error_page") {
		fetchErrorPages(serv._errorPages);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "cgi") {
		fetchCGI(serv._cgi);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else {
		throw ParsingError(UNK_SER_DIR_ERR + cur._token + "\"", _path, cur._line, cur._col);
	}
}

void	ConfigParser::parseLocationDir ( Location& loc )
{
	Token cur = current();

	++_index;
	if (cur._token == "root") {
		fetchPath(loc._path);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "index") {
		fetchPathList(loc._index);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "error_page") {
		fetchErrorPages(loc._errorPages);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "client_max_body_size") {
		fetchBodySize(loc._maxBodySize);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "upload_store") {
		fetchPath(loc._uploadStore);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "allowed_methods") {
		fetchMethods(loc._allowedMethods);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "autoindex") {
		loc._autoIndex = fetchAutoIndex();
		loc._autoIndexSet = true;
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "redirect") {
		fetchRedirect(loc);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "cgi") {
		fetchCGI(loc._cgi);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else {
		throw ParsingError(UNK_LOC_DIR_ERR + cur._token + "\"", _path, cur._line, cur._col);
	}
}

void	ConfigParser::fetchListen( ServerEntry& serv )
{
	Token	cur = current();

	if (!accept(T_STR))
		throw ParsingError(EXP_LISTEN_ERR, _path, cur._line, cur._col);

	str		host, portStr;
	size_t	col = cur._token.find(':');
	if (col == str::npos)
		throw ParsingError(INV_LISTEN_ERR + cur._token, _path, cur._line, cur._col);
	host = cur._token.substr(0, col);
	portStr = cur._token.substr(col + 1);

	if (!validHost(host))
		throw ParsingError(INV_LISTEN_ERR + cur._token, _path, cur._line, cur._col);

	if (validatePort(portStr, cur._line, cur._col, _path)) {
		if (!serv._listenSet) {
			serv._listen.erase(serv._listen.begin());
			serv._listenSet = true;
		}
		serv._listen.insert(std::make_pair(host, portStr));
	}
	// if (!serv._listenSet) {
	// 	serv._listen = cur._token;
	// 	serv._listenSet = true;
	// } else {
	// 	std::cout << "[WARNING]: listen already set to [" << serv._listen << "] :: [" << cur._token << "] will be ignored." << std::endl;
	// }
}

// void	ConfigParser::fetchPortList( ServerEntry& serv, const str& path )
// {
// 	(void) serv;
// 	Token cur = current();
// 	while (cur._type == T_NUM) {
// 		int port = std::atoi(cur._token.c_str());
// 		if (validatePort(port, cur._line, cur._col, path)) {
// 			serv._port.insert(port);
// 			serv._portStr.insert(cur._token);
// 		}
// 		++_index;
// 		cur = current();
// 	}
// }

void	ConfigParser::fetchServerName( ServerEntry& serv )
{
	Token	cur = current();

	if (!accept(T_STR))
		throw ParsingError("[" + cur._token + SERV_NAME_ERR, _path, cur._line, cur._col);
	serv._serverName = cur._token;
}

void	ConfigParser::fetchPath( str& path )
{
	Token	cur = current();
	
	if (!accept(T_STR)) 
		throw ParsingError("expected path", _path, cur._line, cur._col);

	path = cur._token;
}

void	ConfigParser::fetchPathList( std::vector<str>& list )
{
	while (current()._type != T_SEMI) {
		Token cur = current();

		list.push_back(cur._token);
		++_index;
	}
}

bool	ConfigParser::fetchAutoIndex( void )
{
	Token	cur = current();
	bool	value;

	if (!accept(T_STR))
		throw ParsingError(INV_AUTO_IDX, _path, current()._line, current()._col);
	if (cur._token == "on" || cur._token == "true")
		value = true;
	else if (cur._token == "off" || cur._token == "false")
		value = false;
	else
		throw ParsingError(INV_AUTO_IDX + cur._token + AUTO_IDX_ERR, _path, cur._line, cur._col);
	return (value);
}

void	ConfigParser::fetchBodySize( size_t& bodySize )
{
	Token	cur = current();

	if (!accept(T_STR))
		throw ParsingError(MAX_BODY_ERR, _path, cur._line, cur._col);

	str	size  = cur._token;
	size_t	i = 0;
	while (i < size.size() && std::isdigit(size[i]))
		++i;

	if (!i) throw ParsingError(MAX_BODY_ERR, _path, cur._line, cur._col);

	str	nums = size.substr(0, i);
	str	unit = size.substr(i);

	size_t num = std::atol(nums.c_str());
	if (unit.empty() || unit.size() != 1)
		throw ParsingError(MAX_BODY_ERR, _path, cur._line, cur._col);
	char target = std::tolower(unit[0]);
	unsigned long mult;
	if (target == 'k')		mult = M_KILO;
	else if (target == 'm')	mult = M_MEGA;
	else if (target == 'g') mult = M_GEGA;
	else
		throw ParsingError(MAX_BODY_ERR, _path, cur._line, cur._col);
	
	size_t maxSize = std::numeric_limits<size_t>::max();
	if (num > maxSize / mult)
		throw ParsingError(MAX_BODY_ERR, _path, cur._line, cur._col);
	bodySize = num * mult;
}

void	ConfigParser::fetchErrorPages( std::map<int, str>& errors )
{
	Token	cur = current();

	if (!accept(T_NUM))
		throw ParsingError("expectedErrorNumber", _path, cur._line, cur._col);

	str	strCode = cur._token;
	if (strCode.size() != 3)
		throw ParsingError("InvalidErrorNumber", _path, cur._line, cur._col);

	cur = current();
	if (!accept(T_STR))
		throw ParsingError("expectedPath", _path, cur._line, cur._col);

	int code = std::atoi(strCode.c_str());
	errors[code] = cur._token;
}

void	ConfigParser::fetchCGI( CGIEntry& cgi )
{
	Token	cur = current();
	if (!accept(T_STR))
		throw ParsingError(EXP_FILE_EXT, _path, cur._line, cur._col);
		
	str	ext = cur._token;
	if (ext.size() < 2 || ext[0] != '.')
		throw ParsingError(INV_CGI_EXT_ERR, _path, cur._line, cur._col);

	cur = current();
	if (!accept(T_STR))
		throw ParsingError(EXP_CGI_ITR_ERR, _path, cur._line, cur._col);
	str inter = cur._token;
	cgi._extention = ext;
	cgi._interpreter = inter;
}

void	ConfigParser::fetchMethods( std::set<str>& methods )
{
	const char* allowed[] = {"GET", "POST", "DELETE", NULL};
	Token cur = current();
	
	while (cur._type == T_STR) {
		str   method = cur._token;
		bool  ok     = false;

		for (int i = 0; allowed[i]; ++i)
			if (method == allowed[i]) {
				ok = true;
			break ;
		}
		if (!ok)
			throw ParsingError(UNK_METHOD_ERR + method + "\"", _path, cur._line, cur._col);
		else
			methods.insert(method);
		++_index;
		cur = current();
	}
	if (methods.empty())
		throw ParsingError(NUM_METHOD_ERR, _path, cur._line, cur._col);
}

void	ConfigParser::fetchRedirect( Location& loc )
{
	Token	cur = current();

	if (!accept(T_NUM))
		throw ParsingError(EXP_REDIR_CODE, _path, cur._line, cur._col);
		
	if (cur._token.size() != 3 || cur._token[0] != '3')
		throw ParsingError(INV_REDIR_CODE + cur._token + "\"", _path, cur._line, cur._col);

	loc._redirCode = std::atoi(cur._token.c_str());
	cur = current();
	if (!accept(T_STR))
		throw ParsingError(NUM_REDIR_ERR, _path, cur._line, cur._col);

	loc._redirTarget = cur._token;
	loc._redirSet = true;
}
