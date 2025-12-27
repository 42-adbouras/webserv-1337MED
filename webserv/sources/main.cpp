/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 11:20:36 by adbouras          #+#    #+#             */
/*   Updated: 2025/12/24 18:52:03 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/webserv.hpp"

CONSOLE g_console;

int	main( int ac, char** av )
{
	if (ac < 2) {
		std::cerr << USAGE_ERROR << std::endl;
		return (1);
	} try {
		Data	config = getConfig(av[1]);
		
		std::vector<TableOfListen>	tableOfListen;
		SocketManager	socketManager( config, tableOfListen );
		// making a hash-table for all IP:PORT
		socketManager.setTableOfListen( tableOfListen );
		socketManager.hanldVirtualHost( tableOfListen );
		// displayHashTable(tableOfListen);
		socketManager.initSockets();
		displayHashTable(tableOfListen);
		socketManager.listenToPorts();
		signal(SIGINT, signalHandler);
		socketManager.runCoreLoop();
		
	} catch (std::exception& e) {
		std::cerr << RED << e.what() << RESET << std::endl;
		return (1);
	}
	return (0);
}
