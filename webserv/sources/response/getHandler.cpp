#include "../../includes/response.hpp"
#include "../../includes/request.hpp"

int fileStat( const str& src ) {
	struct stat st;

	if (lstat(src.c_str(), &st) != 0) {
		return -1;
	}
	if (S_ISDIR(st.st_mode)) return 0;
	if (S_ISREG(st.st_mode)) return 1;

	return -2;
}

void getIndex( ServerEntry *_srvEntry, Response& response ) {
	str index;
	str s;
	if (!_srvEntry->_index.empty())
		index = *_srvEntry->_index.begin();
	if (index.length())
		s = "." + _srvEntry->_root + "/" + index;
	genResponse(response, s);
}

/* void listDirectory( str& src, Response& response, Request& request ) {
	(void)response;
	DIR* dir = opendir(src.c_str());

	struct Entry {
		str name;
		bool is_dir;
		time_t mtime;
		off_t size;
	};
	std::vector<Entry> entries;

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		str name = entry->d_name;
		str full = request.getPath();
		if (full[full.size()-1] != '/') full += '/';
		full += name;

		struct stat st;
		if (lstat(full.c_str(), &st) != 0) continue;
		entries.push_back(Entry{
			name,
			S_ISDIR(st.st_mode),
			st.st_mtim.tv_sec,
			S_ISDIR(st.st_mode) ? 0 : st.st_size
		});
	}

	closedir(dir);
} */

void getHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src) {
	Location location = getLocation(_srvEntry, request, response);
	if (validateRequest(_srvEntry, request, response, location)) {
		if (request.getPath() == "/") {
			getIndex(_srvEntry, response);
			return;
		}
		std::ifstream file(src.c_str());
		sstream buffer;
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
				std::cout << "auto_index ON" << std::endl;
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
