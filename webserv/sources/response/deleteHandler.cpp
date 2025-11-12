#include "../../includes/response.hpp"
#include "../../includes/request.hpp"

void deleteHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src) {
	Location location = getLocation(_srvEntry, request, response);
	if (validateRequest(_srvEntry, request, response, location)) {
		
	}
		genResponse(response, src);
	
}
