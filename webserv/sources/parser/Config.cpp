/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 17:05:39 by adbouras          #+#    #+#             */
/*   Updated: 2025/11/14 16:54:55 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Config.hpp"
#include <cstddef>

ParsingError::ParsingError( const str& msg, const str& path, const Token& cur )
	: _msg(msg)
	, _line(cur._line)
	, _col(cur._col)
	, _path(path)
{
	std::ostringstream	oss;

	oss << "[ERROR]: configuration file " << _path << "["  \
		<< _line << ":" << _col << "]\n\t➜ " << _msg << ".";
	_what = oss.str();
}

ServerEntry::ServerEntry( void )
	: _listenSet(false), _serverName(""), _root("/html"), _maxBodySize(0), _cltHeadTimeout(DEF_HEADER_TIME_OUT)
	, _cltBodyTimeout(DEF_HEADER_TIME_OUT), _keepAliveTimeout(DEF_HEADER_TIME_OUT)
{
	_listen.insert(std::make_pair("0.0.0.0", "8080"));
}

Location::Location( void )
	: _autoIndexSet(false)
	, _autoIndex(false)
	, _redirSet(false)
	, _isCGI(false)
{
	_allowedMethods.insert("GET");
}

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
	throw ParsingError(err, _path, cur);
}

Data	ConfigParser::parseTokens( void )
{
	Data	data;

	while(!accept(T_EOF)) {
		ServerEntry	serv = parseServerBlock();
		mapLocationsRoot(serv);
		data._servers.push_back(serv);
	}
	return (data);
}

void	ConfigParser::mapLocationsRoot( ServerEntry& serv )
{
	for (size_t i = 0; i < serv._locations.size(); ++i) {
		if (serv._locations[i]._root.empty())
			serv._locations[i]._root = serv._root;
	}
}

str		normPath( const str& path )
{
	str out(path);

	if (path.size() == 1 && path[0] == '/')
		return (out);

	int start = 0, end = path.size();

	while (out[start] && out[start] == '/')
		++start;
	while (end > start && out[end - 1] == '/')
		--end;

	if (start == end)
		return ("/");
	return (out.substr(start, end - start));
}

ServerEntry	ConfigParser::parseServerBlock( void )
{
	Token	cur = current();

	if (!(cur._type == T_STR && cur._token == "server"))
		throw ParsingError(EXP_SERV_DIR, _path, cur);

	++_index;

	cur = current();
	if (!accept(T_LBRACE))
		throw ParsingError(SERV_DIR_ERR, _path, cur);

	ServerEntry	s;

	while (!accept(T_RBRACE)) {
		Token cur = current();

		if (cur._type == T_STR && cur._token == "location") {
			Location loc = parseLocationBlock();
			for (size_t i = 0; i < s._locations.size(); ++i) {
				if (normPath(s._locations[i]._path) == normPath(loc._path))
					throw ParsingError(LOC_DUP_ERR + loc._path + "\"", _path, cur);
			}
			s._locations.push_back(loc);
		} else if (cur._type == T_STR) {
			parseServerDir(s);
		} else {
			throw ParsingError("unknown directive \"" + cur._token + "\"", _path, cur);
		}
	}
	return (s);
}

Location	ConfigParser::parseLocationBlock( void )
{
	Token	token = current();
	++_index;
	Token	tPath = current();
	
	if (!(tPath._type == T_STR && startsWith(tPath._token, "/")))
		throw ParsingError("UnexpectedPathPrefix [" + tPath._token + "]", _path, tPath);

	Location	loc;
	loc._path = tPath._token;
	++_index;
	if (!accept(T_LBRACE))
		throw ParsingError("expecting \"{\" after location directive", _path, current());
	while (!accept(T_RBRACE)) {
		if (!(current()._type == T_STR))
			throw ParsingError("expecting \"location\" directive", _path, current());
		parseLocationDir(loc);
	}
	return (loc);
}

bool	isTimeout( const str& token ) {
	return (token == "client_header_timeout" \
			|| token == "client_body_timeout" \
			|| token == "keepalive_timeout");
}

void	ConfigParser::parseServerDir( ServerEntry& serv )
{
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
	} else if (cur._token == "client_max_body_size") {
		fetchBodySize(serv._maxBodySize);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else if (cur._token == "error_page") {
		fetchErrorPages(serv._errorPages);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} /*else if (cur._token == "cgi") {
		fetchCGI(serv._cgi);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} */
	else if (isTimeout(cur._token)) {
		fetchTimeout(serv, cur._token);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else {
		throw ParsingError(UNK_SER_DIR_ERR + cur._token + "\"", _path, cur);
	}
}

void	ConfigParser::parseLocationDir ( Location& loc )
{
	Token cur = current();

	++_index;
	if (cur._token == "root") {
		fetchPath(loc._root);
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
		fetchCGI(loc);
		expect(T_SEMI, EXPECT_SEMI_ERR);
	} else {
		throw ParsingError(UNK_LOC_DIR_ERR + cur._token + "\"", _path, cur);
	}
}

void	ConfigParser::fetchTimeout( ServerEntry& serv, const str& type )
{
	Token 	cur = current();

	if (!accept(T_NUM))
		throw ParsingError("\"" + type + INV_TIME_OUT + cur._token + "\"", _path, cur);

	size_t timeout = std::atoi(cur._token.c_str());

	if (timeout > MAX_TIMEOUT)
		throw ParsingError("\"" + type + MAX_T_OUT_ERR, _path, cur);
	
	if (type == "client_header_timeout")
		serv._cltHeadTimeout = timeout; 
	else if (type == "client_body_timeout")
		serv._cltBodyTimeout = timeout; 
	else serv._keepAliveTimeout = timeout; 
}

void	ConfigParser::fetchListen( ServerEntry& serv )
{
	Token	cur = current();

	if (!accept(T_STR))
		throw ParsingError(EXP_LISTEN_ERR, _path, cur);

	str		host, portStr;
	size_t	col = cur._token.find(':');
	if (col == str::npos)
		throw ParsingError(INV_LISTEN_ERR + cur._token, _path, cur);
	host = cur._token.substr(0, col);
	portStr = cur._token.substr(col + 1);

	if (!validHost(host))
		throw ParsingError(INV_LISTEN_ERR + cur._token, _path, cur);

	if (validatePort(portStr, cur, _path)) {
		if (!serv._listenSet) {
			serv._listen.erase(serv._listen.begin());
			serv._listenSet = true;
		}
		serv._listen.insert(std::make_pair(host, portStr));
	}
}

void	ConfigParser::printWarning( const str& arg, int line, int col )
{
	std::ostringstream	oss;
	
	oss << YELLOW << "[WARNING]: configuration file " << _path << "["  \
		<< line << ":" << col << "]\n\t➜ " << arg << RESET;
	std::cout << oss.str() << std::endl;
}

void	ConfigParser::fetchServerName( ServerEntry& serv )
{
	Token	cur = current();

	if (!accept(T_STR))
		throw ParsingError("[" + cur._token + SERV_NAME_ERR, _path, cur);
	if (serv._serverName.empty())
		serv._serverName = cur._token;
	else
		printWarning(SERV_NAME_WAR + serv._serverName \
							+ "\".\n\t\t:: [" + cur._token + "] will be ignored." \
							, cur._line, cur._col);
}

void	ConfigParser::fetchPath( str& path )
{
	Token	cur = current();
	
	if (!accept(T_STR)) 
		throw ParsingError("expected path", _path, cur);

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
		throw ParsingError(INV_AUTO_IDX, _path, current());
	if (cur._token == "on" || cur._token == "true")
		value = true;
	else if (cur._token == "off" || cur._token == "false")
		value = false;
	else
		throw ParsingError(INV_AUTO_IDX + cur._token + AUTO_IDX_ERR, _path, cur);
	return (value);
}

void	ConfigParser::fetchBodySize( size_t& bodySize )
{
	Token	cur = current();

	if (!accept(T_STR))
		throw ParsingError(MAX_BODY_ERR, _path, cur);

	str	size  = cur._token;
	size_t	i = 0;
	while (i < size.size() && std::isdigit(size[i]))
		++i;

	if (!i) throw ParsingError(MAX_BODY_ERR, _path, cur);

	str	nums = size.substr(0, i);
	str	unit = size.substr(i);

	size_t num = std::atol(nums.c_str());
	if (unit.empty() || unit.size() != 1)
		throw ParsingError(MAX_BODY_ERR, _path, cur);
	char target = std::tolower(unit[0]);
	unsigned long mult;
	if (target == 'k')		mult = M_KILO;
	else if (target == 'm')	mult = M_MEGA;
	else if (target == 'g') mult = M_GEGA;
	else
		throw ParsingError(MAX_BODY_ERR, _path, cur);
	
	size_t maxSize = std::numeric_limits<size_t>::max();
	if (num > maxSize / mult)
		throw ParsingError(MAX_BODY_ERR, _path, cur);
	bodySize = num * mult;
}

void	ConfigParser::fetchErrorPages( std::map<int, str>& errors )
{
	Token	cur = current();

	if (!accept(T_NUM))
		throw ParsingError(EXP_ERR_PAGE, _path, cur);

	str	strCode = cur._token;
	if (strCode.size() != 3)
		throw ParsingError(INV_ERR_PAGE, _path, cur);

	cur = current();
	if (!accept(T_STR))
		throw ParsingError(ERR_PAGE_PATH, _path, cur);

	int code = std::atoi(strCode.c_str());
	if (errors.count(code)) {
		printWarning(DUP_ERRPAGE_WAR + strCode + "].\n\t\t :: replacing \"" \
					+ errors[code] + "\" with \"" + cur._token + "\"", cur._line, cur._col);
	}
	errors[code] = cur._token;
}

void	ConfigParser::fetchCGI( Location& loc )
{
	Token	cur = current();
	if (!accept(T_STR))
		throw ParsingError(EXP_FILE_EXT, _path, cur);

	loc._isCGI = true;
	loc._cgi.push_back(cur._token);
	// str	ext = cur._token;
	// if (ext.size() < 2 || ext[0] != '.')
	// 	throw ParsingError(INV_CGI_EXT_ERR, _path, cur);

	// cur = current();
	// if (!accept(T_STR))
	// 	throw ParsingError(EXP_CGI_ITR_ERR, _path, cur);
	// str inter = cur._token;
	// cgi._extention = ext;
	// cgi._interpreter = inter;
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
			throw ParsingError(UNK_METHOD_ERR + method + "\"", _path, cur);
		else
			methods.insert(method);
		++_index;
		cur = current();
	}
	if (methods.empty())
		throw ParsingError(NUM_METHOD_ERR, _path, cur);
}

void	ConfigParser::fetchRedirect( Location& loc )
{
	Token	cur = current();

	if (!accept(T_NUM))
		throw ParsingError(EXP_REDIR_CODE, _path, cur);
		
	if (cur._token.size() != 3 || cur._token[0] != '3')
		throw ParsingError(INV_REDIR_CODE + cur._token + "\"", _path, cur);

	loc._redirCode = std::atoi(cur._token.c_str());
	cur = current();
	if (!accept(T_STR))
		throw ParsingError(NUM_REDIR_ERR, _path, cur);

	loc._redirTarget = cur._token;
	loc._redirSet = true;
}
