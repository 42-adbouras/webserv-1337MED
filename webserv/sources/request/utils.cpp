#include "../../includes/request.hpp"
#include <sstream>

bool UriAllowedChars( str& uri ) {
	str allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";

	for (str::size_type i=0; i < uri.length(); ++i) {
		if(allowedChars.find(uri[i]) == str::npos)
			return false;
	}
	return true;
}

str normalizePath( const str& path ) {
	str normalizedPath;
	bool lastSlash = false;

	for (size_t i=0; i<path.length(); ++i) {
		if(path[i] == '/') {
			if (!lastSlash) {
				normalizedPath += path[i];
				lastSlash = true;
			}
		} else {
			normalizedPath += path[i];
			lastSlash = false;
		}
	}
	return normalizedPath;
}

str getSource( Request& request, ServerEntry* _srvEntry, Response& response ) {
	str path = request.getPath();
	Location lct = getLocation(_srvEntry, request, response);
	str location = request.getLocation();
	str root = lct._root;

	if (!root.length())
		root = _srvEntry->_root;
	if (root[root.length() - 1] != '/')
		root += "/";
	if (!location.length())
		return "";
	str::size_type start_pos = 0;
	if (location != "/") {
		while((start_pos = path.find(location, start_pos)) != str::npos) {
			path.replace(start_pos, location.length(), root);
			start_pos += root.length();
		}
	} else {
		path.replace(0, location.length(), root);
	}
	std::deque<str> segments = splitPath(path);
	std::deque<str> ss;
	std::deque<str>::iterator it = segments.begin();
	if (segments.empty())
		return "/";
	while (it != segments.end()) {
		if (*it == "..") {
			if (!ss.empty())
				ss.pop_back();
		} else if (*it != "." && !it->empty())
			ss.push_back(*it);
		++it;
	}
	str source;
	while (!ss.empty()) {
		str segment = ss.front();
		ss.pop_front();
		if (segment != "/")
			segment = "/" + segment;
		source += segment;
	}
	source.insert(0, 1, '.');
	return source;
}

str getHost( const HeadersMap& headers ) {
	HeadersMap::const_iterator it = headers.find("Host");
	if (it != headers.end())
		return it->second;
	return "";
}

bool isNumber(str& s) {
	for (size_t i=0; i<s.length(); ++i) {
		if (!isdigit(s[i]))
			return false;
	}
	return true;
}

str urlDecode( const str& path ) {
	str result;

	for (str::size_type i=0; i<path.size(); ++i) {
		char c = path[i];

		if (c == '%') {
			if (i + 2 < path.size()) {
				str hex = path.substr(i + 1, 2);
				std::istringstream hexStream(hex);
				int value;
				if ((hexStream >> std::hex >> value) && (value >= 0 && value <= 255)) {
					result += static_cast<char>(value);
					i += 2;
				} else {
					result += '%';
				}
			} else {
				result += '%';
			}
		} else if (c == '+') {
			result += ' ';
		} else {
			result += c;
		}
	}
	return result;
}
