#include "../../includes/response.hpp"
#include "../../includes/request.hpp"

void getIndex( ServerEntry *_srvEntry, Response& response ) {
	str index;
	str s;
	if (!_srvEntry->_index.empty())
		index = *_srvEntry->_index.begin();
	if (index.length())
		s = "." + _srvEntry->_root + "/" + index;
	genResponse(response, s);
}

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
	response.addHeaders("Content-Length", iToString(response.getContentLength()));
	response.addHeaders("Content-Type", "text/html");
}

void getHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	(void)client;
	Location location = getLocation(_srvEntry, request, response);
	if (validateRequest(_srvEntry, request, response, location)) {
		if (request.getPath() == "/") {
			getIndex(_srvEntry, response);
			return;
		}

		int type = fileStat(src);		
		if (type == 1) {
			// check if CGI

			genResponse(response, src);
			return;
		} else if (type == 0) {
			str path = request.getPath();
			if (path[path.length() - 1] != '/') {
				location._redirTarget = path += "/";
				location._redirCode = MOVED_PERMANENTLY;
				redirResponse(response, location);
				return;
			}
			if (!location._index.empty()) {
				str s = "." + location._root + "/" + *location._index.begin();
				genResponse(response, s);
				return;
			}
			else if (location._autoIndex) {
				listDirectory(src, response, request, _srvEntry);
				return;
			} else {
				errorResponse(response, FORBIDDEN);
				return;
			}
		} else {
			errorResponse(response, NOT_FOUND);
			return;
		}
	}
}
