#include "../../includes/response.hpp"

void postHandler(ServerEntry *_srvEntry, Request& request, Response& response) {
	(void)_srvEntry;
	(void)request;
	(void)response;
	std::cout << "POST" << std::endl;
}
