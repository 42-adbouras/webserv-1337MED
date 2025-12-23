#include "../../includes/response.hpp"
#include "../../includes/request.hpp"
#include "Client.hpp"

void postHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	Location location = getLocation(request, response);
	if (validateRequest(_srvEntry, request, response, location)) {
		int type = fileStat(src);
		if (response.getStatusCode() != 0) {
		if (!client._multipartParser.getUploadedFiles().empty()) {
			client._multipartParser.cleanupAllFiles();
		} else if (client._uploadFd != -1) {
			close(client._uploadFd);
			client._uploadFd = -1;
			if (isFileExist(client._uploadPath)) {
				if (std::remove(client._uploadPath.c_str()) != 0) {
					getSrvErrorPage(response, _srvEntry, INTERNAL_SERVER_ERROR);
					return;
				}
			}
		}
		getSrvErrorPage(response, _srvEntry, response.getStatusCode());
		return;
		}
		if (!location._uploadStore.empty()) {
			response.setStatus(CREATED);
		} else {
			if (type == 1) {
				if (isCgi(location, src, client, request)) {
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
