/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TypeDefs.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 18:33:17 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/25 18:29:58 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <fstream>

#define PORT_MIN 1024
#define PORT_MAX 65535

#define BLUE	"\033[1;34m"
#define RED		"\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW	"\033[1;33m"
#define RESET	"\033[0m"

#define M_KILO 1024UL
#define M_MEGA 1024UL * 1024UL
#define M_GEGA 1024UL * 1024UL * 1024UL

struct Token;

typedef std::string			str;
typedef std::vector<Token>	TokensVector;
typedef std::set< std::pair<str, str> >	ListenSet;
typedef std::stringstream	sstream;

#define EXPECT_SEMI_ERR	"expression is not terminated by \";\""
#define MAX_BODY_ERR	"\"client_max_body_size\" directive invalid value"
#define EXP_LISTEN_ERR	"expected \"listen\" value"
#define INV_LISTEN_ERR	"\"listen\" directive invalid value "
#define INV_AUTO_IDX	"\"autoindex\" directive invalid value "
#define AUTO_IDX_ERR	" must be <on/off/true/false>"
#define UNK_SER_DIR_ERR	"unknown server directive \""
#define UNK_LOC_DIR_ERR	"unknown location directive \""
#define SERV_NAME_ERR	"] \"server_name\" is invalid"
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