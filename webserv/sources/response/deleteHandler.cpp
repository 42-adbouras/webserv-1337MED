#include "../../includes/response.hpp"
#include "../../includes/request.hpp"

void deleteHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	(void)client;
	Location location = getLocation(_srvEntry, request, response);
	if (validateRequest(_srvEntry, request, response, location)) {
		int type = fileStat(src);
		str path = request.getPath();

		if (type == 1) {
			if (isFileExist(src)) {
				remove(src.c_str());
				response.setStatus(NO_CONTENT);
				return;
			}
		} else if (type == 0) {
			if (path[path.length() - 1] != '/') {
				errorResponse(response, CONFLICT);
				return;
			} else {
				
			}
		} else {
			errorResponse(response, NOT_FOUND);
			return;
		}
	}
}
