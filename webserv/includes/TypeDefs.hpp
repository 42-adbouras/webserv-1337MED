/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TypeDefs.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 18:33:17 by adbouras          #+#    #+#             */
/*   Updated: 2025/11/07 14:56:38 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <fstream> // IWYU pragma: keep
#include <map>
#include <cstddef>
#include <exception> // IWYU pragma: keep
#include <cstdlib>
#include <limits> // IWYU pragma: keep

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
#define RESET	"\033[0m"

#define PORT_MIN 1024
#define PORT_MAX 65535

#define MAX_TIMEOUT 86400

#define M_KILO 1024UL
#define M_MEGA 1024UL * 1024UL
#define M_GEGA 1024UL * 1024UL * 1024UL

struct CGIEntry
{
	str						_extention;
	str						_interpreter;
};

struct Location
{
	str						_path;
	str						_root;
	std::vector<str>		_index;
	std::map<int, str>		_errorPages;
	size_t					_maxBodySize;
	str						_uploadStore;
	std::set<str>			_allowedMethods;

	bool					_autoIndexSet;
	bool					_autoIndex;
	
	bool					_redirSet;
	int						_redirCode;
	str						_redirTarget;
	CGIEntry				_cgi;
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
	size_t					_cltHeadTimeout;
	size_t					_cltBodyTimeout;
	size_t					_keepAliveTimeout;
	CGIEntry				_cgi;
	std::vector<Location>	_locations;
	ServerEntry( void );
};


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