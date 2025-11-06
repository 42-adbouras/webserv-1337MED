#include "../../includes/response.hpp"

void getHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src) {
	(void)src;
	(void)_srvEntry;
	(void)request;
	(void)response;
	std::cout << "GET" << std::endl;
}