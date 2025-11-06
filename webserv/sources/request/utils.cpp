#include "../../includes/request.hpp"

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
	str::size_type start_pos = 0;
	str location = request.getLocation();
	std::cout << "locationnnnn: " << location << std::endl;
	str root = lct._root;
	if (!root.length())
		root = _srvEntry->_root;
	if (root[root.length() - 1] != '/')
		root += "/";
	if (!location.length())
		return "";
	if (location != "/") {
		while((start_pos = path.find(location, start_pos)) != str::npos) {
			path.replace(start_pos, location.length(), root);
			start_pos += root.length();
		}
	}
	std::deque<str> segments = splitPath(path);
	std::deque<str> ss;
	str source;
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

str getHost( const std::map<str, str>& headers ) {
	std::map<str, str>::const_iterator it = headers.find("Host");
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

str fileOpen( const str& source ) {
	std::ifstream file(source.c_str());
	sstream buffer;
	if (!file.is_open())
		return "";
	else
		buffer << file.rdbuf();
	return buffer.str();
}