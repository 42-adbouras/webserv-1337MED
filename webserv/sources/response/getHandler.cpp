#include "../../includes/response.hpp"
#include "../../includes/request.hpp"
#include "Client.hpp"

void listDirectory( str& src, Response& response, Request& request, ServerEntry* _srvEntry ) {
	struct dirent *entry;
	std::ostringstream html;
	html << "<!DOCTYPE html>\n"
		 << "<html>\n"
		 << "<head><title>Index of " << request.getPath() << "</title></head>\n"
		 << "<body>\n"
		 << "<h1>Index of " << request.getPath() << "</h1>\n"
		 << "<ul>\n";

	DIR* dir = opendir(src.c_str());
	str path = request.getPath();
	if (dir == NULL) {
		html << "<li>Cannot open directory.</li>\n";
		closedir(dir);
	} else {
		while ((entry = readdir(dir)) != NULL) {
			str name = entry->d_name;
			if (name == "." || name == "..") continue;
			str full = "." + _srvEntry->_root + path + "/" + name;
			struct stat st;
			bool is_dir = false;

			if (stat(full.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
				is_dir = true;

			html << "  <li><a href=\"" << path;
			if (path[path.size() - 1] != '/')
				html << "/";
			html << name;
			if (is_dir)
				html << "/";
			html << "\">" << name;
			if (is_dir)
				html << "/";
			html << "</a></li>\n";
		}
		closedir(dir);
	}

	html << "</ul>\n</body>\n</html>\n";
	str responseHtml = html.str();
	response.setBody(responseHtml);
	response.setStatus(OK);
	response.addHeaders("Content-Type", "text/html; charset=UTF-8");
}

void getIndex( ServerEntry *_srvEntry, Response& response, Location& lct, Request& request ) {
	str index;
	str s;
	if (!_srvEntry->_index.empty()) {
		std::vector<str>::iterator it = _srvEntry->_index.begin();
		while (it != _srvEntry->_index.end()) {
			index = *it;
			s = "." + _srvEntry->_root + "/" + index;
			if (isFileExist(s)) {
				genResponse(response, s, _srvEntry);
				return;
			}
			++it;
		}
	} else if (lct._autoIndex) {
		str src = "." + _srvEntry->_root;
		listDirectory(src, response, request, _srvEntry);
	} else {
		genResponse(response, s, _srvEntry);
	}
}

CGIContext fillCgiContext( str& src, Request& request, str& name, Client& client ) {
	CGIContext cgiContext = client.getCgiContext();
	cgiContext._path = src;
	cgiContext._name = name;
	cgiContext._body = request.getBody();
	cgiContext._method = request.getMethod();
	cgiContext._serverName = client.getRequest().getSrvEntry()->_serverName;
	cgiContext._query = request.getQueryParams();
	cgiContext._headers = request.getHeaders();
	return cgiContext;
}

bool isCgi( Location& location, str& src, Client& client, Request& request ) {
	if (location._isCGI) {
		if (location._cgi.empty())
			return false;
		str cgiFile = src.substr(src.find_last_of("/") + 1);
		std::vector<str>::iterator it = location._cgi.begin();
		while (it != location._cgi.end()) {
			if (*it == cgiFile) {
				CGIContext cgiContext = fillCgiContext(src, request, cgiFile, client);
				client.setCgiContext(cgiContext);
				break;
			}
			++it;
		}
		if (it != location._cgi.end())
			return true;
	}
	return false;
}

void getHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	Location location = getLocation(request, response);
	HeadersMap hdrs = request.getHeaders();
	if (validateRequest(_srvEntry, request, response, location)) {
		if (request.getPath() == "/") {
			getIndex(_srvEntry, response, location, request);
			return;
		}

		int type = fileStat(src);
		if (type == 1) {
			if (isCgi(location, src, client, request)) {
				client.setClientState(CS_CGI_REQ);
				return;
			}
			if (getContentType(src) == "video/mp4"
				|| getContentType(src) == "audio/mpeg"
				|| getContentType(src) == "video/x-matroska"
				|| getContentType(src) == "video/mpeg"
				|| getContentType(src) == "video/webm"
				|| getContentType(src) == "video/x-msvideo"
				|| getContentType(src) == "audio/wav"
				|| getContentType(src) == "audio/ogg") {
				response.setFlag(true);
			}
			genResponse(response, src, _srvEntry);
			return;
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
			}
			else if (location._autoIndex) {
				listDirectory(src, response, request, _srvEntry);
				return;
			} else {
				getSrvErrorPage(response, _srvEntry, FORBIDDEN);
				return;
			}
		} else {
			getSrvErrorPage(response, _srvEntry, NOT_FOUND);
			return;
		}
	}
}
