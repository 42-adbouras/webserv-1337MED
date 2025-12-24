/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:43 by adbouras          #+#    #+#             */
/*   Updated: 2025/08/23 16:07:39 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "serverHeader/Server.hpp"
#include "Lexer.hpp"
#include "TypeDefs.hpp"
#include "Config.hpp"
#include "serverHeader/SocketManager.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include "CGI.hpp"
#include "serverHeader/ServerUtils.hpp"