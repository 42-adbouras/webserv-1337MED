#include "../../includes/response.hpp"
#include "../../includes/request.hpp"
#include "Client.hpp"

void postHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	(void)client;
	(void)src;
	(void)_srvEntry;
	(void)request;
	Location location = getLocation(_srvEntry, request, response);
	if (validateRequest(_srvEntry, request, response, location)) {
		int type = fileStat(src);
		if (type == 1) {
			// to-do
		}
	}
}
