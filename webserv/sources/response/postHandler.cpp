#include "../../includes/response.hpp"

void postHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	(void)client;
	(void)src;
	(void)_srvEntry;
	(void)request;
	(void)response;
	std::cout << "POST" << std::endl;
}
