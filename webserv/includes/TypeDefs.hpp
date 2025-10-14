/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TypeDefs.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 18:33:17 by adbouras          #+#    #+#             */
/*   Updated: 2025/09/23 15:25:10 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>

#define PORT_MIN 1
#define PORT_MAX 65535

#define M_KILO 1024UL
#define M_MEGA 1024UL * 1024UL
#define M_GEGA 1024UL * 1024UL * 1024UL

struct Token;

typedef std::string			str;
typedef std::vector<Token>	TokensVector;
