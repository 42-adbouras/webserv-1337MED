#include "../../includes/response.hpp"

void deleteHandler(ServerEntry *_srvEntry, Request& request, Response& response) {
	str source = request.getPath();
	// std::cout << source << std::endl;
	std::deque<str> segments = splitPath(source);
	Location location = getLocation(_srvEntry, request);
	if (location._allowedMethods.find(request.getMethod())
		== location._allowedMethods.end())
		errorResponse(response, METHOD_NOT_ALLOWED);
	if (request.getBody().length()) {
		if (request.getBody().length() > _srvEntry->_maxBodySize)
			errorResponse(response, CONTENET_TOO_LARGE);
	}
	// std::deque<str>::iterator it = segments.begin();
	// while(it != segments.end()) {
	// 	std::cout << *it << std::endl;
	// 	++it;
	// }
}
