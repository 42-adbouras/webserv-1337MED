#include "../includes/Utils.hpp"
#include "../includes/TypeDefs.hpp"

int	setNonBlocking( int fd )
{
	return (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK));
}


// std::string	iToString(int x) {
// 	std::string	str;
// 	while (x > 0)
// 	{
// 		char c = x % 10 + '0';
// 		str.insert(str.begin(), c);
// 		x /= 10;
// 	}
// 	return str;
// }

str iToString(int n) {	
	sstream ss;
	ss << n;
	return ss.str();
}