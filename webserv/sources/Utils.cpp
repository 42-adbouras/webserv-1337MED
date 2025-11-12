#include "../includes/Utils.hpp"
#include "../includes/TypeDefs.hpp"

int	setNonBlocking( int fd )
{
	return (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK));
}

str iToString(size_t n) {	
	sstream ss;
	ss << n;
	return ss.str();
}