#include "Utils.hpp"

int	setNonBlocking( int fd )
{
	return (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK));
}
