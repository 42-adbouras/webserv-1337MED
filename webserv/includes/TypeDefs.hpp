/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TypeDefs.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 18:33:17 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/24 10:49:09 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <set>

#define BLUE	"\033[1;34m"
#define RED		"\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW	"\033[1;33m"
#define RESET	"\033[0m"

#define PORT_MIN 1024
#define PORT_MAX 65535

#define M_KILO 1024UL
#define M_MEGA 1024UL * 1024UL
#define M_GEGA 1024UL * 1024UL * 1024UL

struct Token;

typedef std::string						str;
typedef std::vector<Token>				TokensVector;
typedef std::set< std::pair<str, str> >	ListenSet;

#define EXPECT_SEMI_ERR	"expected \";\" after expression"
#define MAX_BODY_ERR	"\"client_max_body_size\" directive invalid value"
#define EXP_LISTEN_ERR	"expected \"listen\" value"
#define INV_LISTEN_ERR	"\"listen\" directive invalid value "
#define INV_AUTO_IDX	"\"autoindex\" directive invalid value "
#define AUTO_IDX_ERR	" must be <on/off/true/false>"
#define UNK_SER_DIR		"unknown server directive \""
