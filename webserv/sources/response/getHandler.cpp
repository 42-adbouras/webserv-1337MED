#include "../../includes/response.hpp"

void getHandler(ServerEntry *_srvEntry, Request& request, Response& response) {
	(void)_srvEntry;
	(void)request;
	(void)response;
	std::cout << "GET" << std::endl;
}