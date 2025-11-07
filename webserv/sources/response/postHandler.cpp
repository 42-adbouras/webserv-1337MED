#include "../../includes/response.hpp"

void postHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src) {
	(void)src;
	(void)_srvEntry;
	(void)request;
	(void)response;
	std::cout << "POST" << std::endl;
}
