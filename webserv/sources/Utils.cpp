#include "../includes/Utils.hpp"
#include "../includes/TypeDefs.hpp"

int	setNonBlocking( int fd )
{
	return (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK));
}

str iToString(int n) {	
	sstream ss;
	ss << n;
	return ss.str();
}