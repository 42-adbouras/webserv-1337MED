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


str iToString(size_t n) {	
	sstream ss;
	ss << n;
	return ss.str();
}