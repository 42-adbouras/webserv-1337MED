#include "../../includes/response.hpp"
#include "../../includes/request.hpp"
#include "Client.hpp"

void getIndex( ServerEntry *_srvEntry, Response& response ) {
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
	}
	genResponse(response, s, _srvEntry);
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
	response.addHeaders("Content-Type", "text/html; charset=UTF-8");
}

CGIContext fillCgiContext( str& src, Request& request, ServerEntry* _srvEntry, str& name, Client& client ) {
	CGIContext cgiContext = client.getCgiContext();
	cgiContext._path = src;
	cgiContext._name = name;
	cgiContext._body = request.getBody();
	cgiContext._method = request.getMethod();
	cgiContext._serverName = _srvEntry->_serverName;
	cgiContext._query = request.getQueryParams();
	cgiContext._headers = request.getHeaders();
	return cgiContext;
}

bool isCgi( Location& location, str& src, Client& client, ServerEntry *_srvEntry, Request& request ) {
	if (location._isCGI) {
		if (location._cgi.empty())
			return false;
		str cgiFile = src.substr(src.find_last_of("/") + 1);
		std::vector<str>::iterator it = location._cgi.begin();
		while (it != location._cgi.end()) {
			if (*it == cgiFile) {
				CGIContext cgiContext = fillCgiContext(src, request, _srvEntry, cgiFile, client);
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


// struct Range {
// 	long long start;
// 	long long end;
// 	bool valid;
// 	str error;
// };

// Range parseRangeHdr( const str& range, long long fileSize ) {
// 	Range r = {-1, -1, false, ""};

// 	if (range.size() < 7 || range.substr(0, 6) != "bytes=") {
// 		r.error = "Not Bytes=";
// 		return r;
// 	}

// 	str rangeSpec = range.substr(6);

// 	size_t dash_pos = rangeSpec.find('-');
// 	if (dash_pos == str::npos) {
// 		r.error = "No dash found";
// 		return r;
// 	}

// 	str first = rangeSpec.substr(0, dash_pos);
// 	str second = rangeSpec.substr(dash_pos + 1);

// 	char* endPtr;
// 	if (first.empty()) {
// 		r.start = -1;
// 	} else {
// 		long long val = strtoll(first.c_str(), &endPtr, 10);
// 		if (*endPtr != '\0' || val < 0) {
// 			r.error = "Invalid start";
// 			return r;
// 		}
// 		r.start = val;
// 	}
// 	if (second.empty()) {
// 		r.end = -1;
// 	} else {
// 		long long val = strtoll(second.c_str(), &endPtr, 10);
// 		if (*endPtr != '\0' || val < 0) {
// 			r.error = "Invalid end";
// 			return r;
// 		}
// 		r.end = val;
// 	}

// 	if (r.start != -1 && r.end != -1 && r.start > r.end)
// 	{
// 		r.error = "start > end";
// 		return r;
// 	}

// 	if (r.start != -1 && r.end != -1) {
// 		if (r.end <= 0 || r.end > fileSize) {
// 			r.error = "Invalid Suffix Length";
// 			return r;
// 		}
// 		r.start = fileSize - r.end;
// 		r.end = fileSize - 1;		
// 	}

// 	if (r.start != -1 && r.start >= fileSize) {
// 		r.valid = false;
// 		r.error = "Start Beyond EOF";
// 		return r;
// 	}

// 	if (r.end == -1)
// 		r.end = fileSize - 1;
// 	if (r.start == -1)
// 		r.start = 0;
	
// 	r.start = std::max(0LL, std::min(r.start, fileSize - 1));
// 	r.end = std::min(r.end, fileSize - 1);

// 	r.valid = true;
// 	return r;
// }

// long long getFileSize( const str& src ) {
// 	struct stat st;

// 	if (stat(src.c_str(), &st) == -1)
// 		return -1;
// 	if (!S_ISREG(st.st_mode))
// 		return -1;

// 	return (long long)st.st_size;
// }

// template<typename T>
// str toString(T n) {
// 	std::ostringstream ss;
// 	ss << n;
// 	return ss.str();
// }

// bool send_file_chunk(int client_socket, const std::string& filepath, long long start, long long end)
// {
//     int fd = open(filepath.c_str(), O_RDONLY);
//     if (fd == -1) {
//         // File not found or permission denied
//         return false;
//     }

//     // Seek to the start position
//     if (lseek(fd, start, SEEK_SET) == (off_t)-1) {
//         close(fd);
//         return false;
//     }

//     long long bytes_to_send = end - start + 1;
//     long long bytes_sent_total = 0;

//     const int CHUNK_SIZE = 8192;  // 8 KB → perfect balance (don't change!)
//     char buffer[CHUNK_SIZE];

//     while (bytes_sent_total < bytes_to_send)
//     {
//         long long remaining = bytes_to_send - bytes_sent_total;
//         int to_read = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : (int)remaining;

//         ssize_t bytes_read = read(fd, buffer, to_read);
//         if (bytes_read <= 0) {
//             // EOF or error
//             if (bytes_read < 0) {
//                 // real error
//                 close(fd);
//                 return false;
//             }
//             break; // EOF (should not happen if sizes are correct)
//         }

//         // Send the chunk
//         ssize_t bytes_sent = send(client_socket, buffer, bytes_read, 0);
//         if (bytes_sent != bytes_read) {
//             // Connection closed or error
//             close(fd);
//             return false;
//         }

//         bytes_sent_total += bytes_sent;
//     }

//     close(fd);
//     return true;
// }

// void send_response_headers(int client_socket, const Response& resp)
// {
//     std::string headers = resp.generate();  // your generate() returns full headers + \r\n\r\n
//     send(client_socket, headers.c_str(), headers.length(), 0);
// }

void getHandler(ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client) {
	Location location = getLocation(_srvEntry, request, response);
	response.addHeaders("Accept-Ranges", "bytes");
	HeadersMap hdrs = request.getHeaders();
	if (validateRequest(_srvEntry, request, response, location)) {
		if (request.getPath() == "/") {
			getIndex(_srvEntry, response);
			return;
		}

		int type = fileStat(src);
		if (type == 1) {
			if (isCgi(location, src, client, _srvEntry, request)) {
				client.setClientState(CS_CGI_REQ);
				// response.setBody("CGI");
				// response.setStatus(OK);
				// response.addHeaders("Content-Length", iToString(response.getContentLength()));
				// response.addHeaders("Content-Type", "text/plain");
				return;
			} else {
				if (getContentType(src) == "video/mp4" || getContentType(src) == "audio/mpeg") {
					response.setFlag(true);
					return;
					// HeadersMap::iterator rng = hdrs.find("Range");
					// if (rng != hdrs.end()) {
					// 	str bytesRange = rng->second;
					// 	long long fileSize = getFileSize(src);
					// 	Range range = parseRangeHdr(bytesRange, fileSize);
					// 	if (!range.valid) {
					// 		response.addHeaders("Content-Range", "bytes */" + toString<long long>(fileSize));
					// 		getSrvErrorPage(response, _srvEntry, RANGE_NOT_SATISFIABLE);
					// 		return;
					// 	}
					// 	response.setStatus(PARTIAL_CONTENT);
					// 	response.addHeaders("Content-Range", "byptes" + toString<long long>(range.start) + "-" + toString<long long>(range.end) + "/" + toString<long long>(fileSize));
					// 	response.addHeaders("Content-Length", iToString(range.end - range.start + 1));
					// 	response.addHeaders("Content-Type", getContentType(src));
					// 	send_response_headers(client.getFd(), response);
					// 	if (!send_file_chunk(client.getFd(), src, range.start, range.end)) {
					// 		std::cout << "WIEIWIEIEWIWEIEIIWE" << std::endl;
					// 	}
					// 	return;
					// }
				}
				genResponse(response, src, _srvEntry);
			}
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
