#include "../includes/Utils.hpp"
#include "../includes/TypeDefs.hpp"

/* Display all socket on poll() */
void    displayPOllList(const std::vector<pollfd>& list) {
    std::cout << CYAN << "Open Socket List:" << RESET << std::endl;
    for (size_t i = 0; i < list.size(); i++)
    {
        std::cout << list[i].fd;
        if (i + 1 < list.size())
            std::cout << "--";
    }
    std::cout << GREEN << " -|" << RESET << std::endl;
}

/* Display Tables Of listen Socket information */
void	displayHashTable(const std::vector<TableOfListen> &table) {
	for (size_t i = 0; i < table.size(); i++)
	{
		std::cout << YELLOW;
		std::cout << "TABLE " << i + 1 << ": ==> " << "[ FD=" << table[i]._fd << ", IP=" << table[i]._ip << ", PORT=" << table[i]._port << ", SERVER_NAME=" << table[i]._serverName << " ]" << std::endl;
		std::cout << GREEN << "          ========================        " << std::endl;
	}
	std::cout << RESET << std::endl;
}

str iToString(size_t n) {	
	sstream ss;
	ss << n;
	return ss.str();
}
