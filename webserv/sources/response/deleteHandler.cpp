#include "../../includes/response.hpp"
#include "../../includes/request.hpp"
#include "Client.hpp"

void deleteHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	(void)client;
	Location location = getLocation(_srvEntry, request, response);
	if (validateRequest(_srvEntry, request, response, location)) {
		int type = fileStat(src);
		str path = request.getPath();

		if (type == 1) {
			if (isCgi(location, src, client, _srvEntry)) {
				client.setClientState(CS_CGI_REQ);
				return;
			}
			else if (isFileExist(src)) {
				if (std::remove(src.c_str()) != 0) {
					std::strerror(errno);
					errorResponse(response, FORBIDDEN);
				}
				else
					response.setStatus(NO_CONTENT);
				return;
			}
		} else if (type == 0) {
			if (path[path.length() - 1] != '/') {
				errorResponse(response, CONFLICT);
				return;
			} else {
				if (location._isCGI) {
					if (location._index.empty()) {
						errorResponse(response, FORBIDDEN);
						return;
					} else {
						str source = *location._index.begin();
						if (isCgi(location, source, client, _srvEntry)) {
							client.setClientState(CS_CGI_REQ);
							return;
						}
					}
				} else {
					if (std::remove(src.c_str()) != 0) {
						std::strerror(errno);
						if (errno == EACCES || errno == EPERM)
							errorResponse(response, FORBIDDEN);
						else
							errorResponse(response, INTERNAL_SERVER_ERROR);
					}
					else
						response.setStatus(NO_CONTENT);
				}
			}
		} else {
			errorResponse(response, NOT_FOUND);
			return;
		}
	}
}
