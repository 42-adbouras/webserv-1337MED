#include "../../includes/response.hpp"
#include "../../includes/request.hpp"
#include "Client.hpp"

void postHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	/* if (!request.getBody().length()) {
		getSrvErrorPage(response, _srvEntry, BAD_REQUEST);
		return;
	} */
	Location location = getLocation(_srvEntry, request, response);
	if (validateRequest(_srvEntry, request, response, location)) {
		int type = fileStat(src);
		if (!location._uploadStore.empty()) {
			response.setStatus(CREATED);
		} else {
			if (type == 1) {
				if (isCgi(location, src, client, _srvEntry, request)) {
					client.setClientState(CS_CGI_REQ);
					return;
				}
				getSrvErrorPage(response, _srvEntry, FORBIDDEN);
			} else if (type == 0) {
				str path = request.getPath();
				if (path[path.length() - 1] != '/') {
					location._redirTarget = path += "/";
					location._redirCode = MOVED_PERMANENTLY;
					redirectionResponse(response, location);
					return;
				}
				if (!location._index.empty()) {
					str s = "." + location._root + "/" + *location._index.begin();
					genResponse(response, s, _srvEntry);
					return;
				} else {
					getSrvErrorPage(response, _srvEntry, FORBIDDEN);
					return;
				}
			} else {
				getSrvErrorPage(response, _srvEntry, FORBIDDEN);
				return;
			}
		}
	}
}
