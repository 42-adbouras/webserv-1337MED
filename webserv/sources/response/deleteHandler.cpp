#include "../../includes/response.hpp"
#include "../../includes/request.hpp"
#include "Client.hpp"

void deleteHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	Location location = getLocation(_srvEntry, request, response);
	if (validateRequest(_srvEntry, request, response, location)) {
		int type = fileStat(src);
		str path = request.getPath();

		if (type == 1) {
			if (isCgi(location, src, client, _srvEntry, request)) {
				client.setClientState(CS_CGI_REQ);
				return;
			}
			else if (isFileExist(src)) {
				if (std::remove(src.c_str()) != 0) {
					std::strerror(errno);
					if (errno == EACCES || errno == EPERM) {
						getSrvErrorPage(response, _srvEntry, FORBIDDEN);
					}
					else
						getSrvErrorPage(response, _srvEntry, INTERNAL_SERVER_ERROR);
				}
				else
					response.setStatus(NO_CONTENT);
				return;
			}
		} else if (type == 0) {
			if (path[path.length() - 1] != '/') {
				getSrvErrorPage(response, _srvEntry, CONFLICT);
				return;
			} else {
				getSrvErrorPage(response, _srvEntry, FORBIDDEN);
			}
		} else {
			getSrvErrorPage(response, _srvEntry, NOT_FOUND);
			return;
		}
	}
}
