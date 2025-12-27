/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TypeDefs.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 18:33:17 by adbouras          #+#    #+#             */
/*   Updated: 2025/12/27 16:08:59 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <fstream>
#include <map>
#include <cstddef>
#include <exception>
#include <cstdlib>
#include <limits>

#include <cctype>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <sys/time.h>

struct Token;
typedef std::string						str;
typedef std::vector<Token>				TokensVector;
typedef std::set< std::pair<str, str> >	ListenSet;
typedef std::stringstream				sstream;
typedef std::map<str, str>				HeadersMap;
typedef std::map<str, str>				QueryMap;

#define BLUE	"\033[1;34m"
#define RED		"\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW	"\033[1;33m"
#define CYAN	"\033[1;36m"
#define ORANGE	"\033[1;37m"
#define RESET	"\033[0m"

// Background Color

#define BLUE	"\033[1;34m"
#define RED		"\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW	"\033[1;33m"
#define CYAN	"\033[1;36m"
#define WHITE	"\033[1;37m"
#define MAGENTA	"\033[1;38m"
#define RESET	"\033[0m"

// Background Color

#define BG_BLUE		"\033[44m"
#define BG_GREEN	"\033[42m"
#define BG_RED	"\033[41m"
#define BG_CYAN	"\033[47m"

#define PORT_MIN 1024
#define PORT_MAX 65535

#define MAX_TIMEOUT 86400

#define M_KILO 1024UL
#define M_MEGA 1024UL * 1024UL
#define M_GEGA 1024UL * 1024UL * 1024UL

#define	SRV_READ_BUFFER 4096 // => 4KB
#define	SRV_SEND_BUFFER 4096 // => 4KB
#define CGI_SEND_BUFFER 4096 // => 4KB
#define CGI_R_BUFFER 4096 // => 4KB

#define HEADER_TIME_OUT	10 //s
#define CGI_TIME_OUT	10 //s

struct Location
{
	str						_path;
	str						_root;
	std::vector<str>		_index;
	std::map<int, str>		_errorPages;
	size_t					_maxBodySize;
	str						_uploadStore;
	bool					_methodSet;
	std::set<str>			_allowedMethods;

	bool					_autoIndexSet;
	bool					_autoIndex;
	
	bool					_redirSet;
	int						_redirCode;
	str						_redirTarget;

	bool					_isCGI;
	std::vector<str>		_cgi;

	Location( void );
};

struct ServerEntry
{
	ListenSet				_listen;
	bool					_listenSet;
	str						_serverName;
	str						_root;
	std::vector<str>		_index;
	std::map<int, str>		_errorPages;
	size_t					_maxBodySize;
	str						_uploadStore;
	size_t					_headerTimeout;
	size_t					_cgiTimeout;
	size_t					_keepAliveTimeout;
	std::vector<Location>	_locations;
	ServerEntry( void );
};

enum	Status {    // Enum for programe status only.
	S_TIMEDOUT,
    S_CONTINUE,
	NON
};

#define USAGE_ERROR		"Usage: ./webserv <config/path.conf>"
#define INV_CFG_PATH	"[ERROR]: invalid config path exception."
#define INV_CFG_FILE	"[ERROR]: invalid config file exception."
#define FAIL_OPEN_FILE	"[ERROR]: failed to open file exception."
#define FAIL_READ_FILE	"[ERROR]: failed to read file exception."

#define EXPECT_SEMI_ERR	"expression is not terminated by \";\""
#define MAX_BODY_ERR	"\"client_max_body_size\" directive invalid value"
#define EXP_LISTEN_ERR	"expected \"listen\" value"
#define INV_LISTEN_ERR	"\"listen\" directive invalid value "
#define INV_AUTO_IDX	"\"autoindex\" directive invalid value "
#define AUTO_IDX_ERR	" must be <on/off/true/false>"
#define UNK_SER_DIR_ERR	"unknown server directive \""
#define UNK_LOC_DIR_ERR	"unknown location directive \""
#define SERV_NAME_ERR	"] \"server_name\" is invalid"
#define SERV_NAME_WAR	"\"server_name\" directive already set to \""
#define UNK_METHOD_ERR	"unsupported method \""
#define NUM_METHOD_ERR	"invalid number of arguments after \"allowed_methods\""
#define EXP_REDIR_CODE	"expecting \"redirect\" code after expretion"
#define INV_REDIR_CODE	"invalide \"redirect\" code \""
#define NUM_REDIR_ERR	"invalid number of arguments in \"rerirect\" directive"
#define EXP_FILE_EXT	"expecting file extention after expression \"cgi\""
#define INV_CGI_EXT_ERR	"invalid extention in \"cgi\" directive"
#define EXP_CGI_ITR_ERR	"expecting interpreter after extention"
#define EXP_SERV_DIR	"expecting \"server\" directive"
#define SERV_DIR_ERR	"\"server\" directive has no opening \"{\""
#define EXP_ERR_PAGE	"expecting error number after \"error_page\" directive"
#define INV_ERR_PAGE	"invalid error number in \"error_page\" directive"
#define ERR_PAGE_PATH	"expecting path in \"error_page\" directive"
#define INV_TIME_OUT	"\" directive invalid value \""
#define MAX_T_OUT_ERR	"\" directive must be less than 24 hours"
#define DUP_ERRPAGE_WAR	"duplicated \"error_page\" for ["
#define LOC_DUP_ERR		"detected \"location\" directive duplication \""
#define EXP_PATH_PREFIX	"unexpected path prefix ["
#define EXP_L_BRACKET	"expecting \"{\" after location directive"
#define EXP_LOC_DIR		"expecting \"location\" directive"
#define UNK_DIRECTIVE	"unknown directive \""
#define EXP_PATH_START	"expected path to starts with '/' on ["
