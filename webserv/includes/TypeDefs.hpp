/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TypeDefs.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abmahfou <abmahfou@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 18:33:17 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/11 11:04:08 by abmahfou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#define PORT_MIN 1
#define PORT_MAX 65535

#define M_KILO 1024UL
#define M_MEGA 1024UL * 1024UL
#define M_GEGA 1024UL * 1024UL * 1024UL

struct Token;

typedef std::string			str;
typedef std::vector<Token>	TokensVector;
typedef std::stringstream	sstream;
