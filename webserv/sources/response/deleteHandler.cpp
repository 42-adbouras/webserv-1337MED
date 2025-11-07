#include "../../includes/response.hpp"
#include "../../includes/request.hpp"

void redirResponse( Response& response, Location location ) {
	response.setClientState(location._redirCode);
	response.addHeaders("Location", location._redirTarget);
	response.addHeaders("Content-Length", iToString(response.getContentLength()));

	response.setBody("Moved Permanently. Redirecting to " + location._redirTarget);
}

void deleteHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src) {
	(void)src;
	Location location = getLocation(_srvEntry, request, response);
	if (request.getBody().length()) {
		if (request.getBody().length() > _srvEntry->_maxBodySize)
			errorResponse(response, CONTENET_TOO_LARGE);
	if (location._allowedMethods.find(request.getMethod())
		== location._allowedMethods.end())
		errorResponse(response, METHOD_NOT_ALLOWED);
	}
	if (location._redirSet) {
		redirResponse(response, location);
		return;
	}
}
