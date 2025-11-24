#include "../../includes/response.hpp"
#include "../../includes/request.hpp"

static str toLower( const str& s ) {
	str out = s;
	std::transform(out.begin(), out.end(), out.begin(), static_cast<int(*)(int)>(std::tolower));
	return out;
}

static std::map<str, str> mimeMap() {
	std::map<str, str> m;
	if (m.empty()) {
		m[".html"] = "text/html; charset=UTF-8";
		m[".htm"]  = "text/html";
		m[".css"]  = "text/css";
		m[".py"]   = "text/x-python";
		m[".php"]  = "application/x-httpd-php";
		m[".js"]   = "application/javascript";
		m[".json"] = "application/json";
		m[".xml"]  = "application/xml";
		m[".txt"]  = "text/plain";
		m[".png"]  = "image/png";
		m[".jpg"]  = "image/jpeg";
		m[".jpeg"] = "image/jpeg";
		m[".gif"]  = "image/gif";
		m[".svg"]  = "image/svg+xml";
		m[".pdf"]  = "application/pdf";
		m[".zip"]  = "application/zip";
		m[".mp3"]  = "audio/mpeg";
		m[".mp4"]  = "video/mp4";
		m[".mkv"] = "video/x-matroska";
		m[".woff"] = "application/font-woff";
		m[".woff2"]= "application/font-woff2";
	}
	return m;
}

str getContentType( const str& path ) {
	str::size_type dot = path.find_last_of('.');
	str ext;

	if (dot != str::npos) {
		ext = path.substr(dot);
		ext = toLower(ext);
	}
	const std::map<str, str>& m = mimeMap();
	std::map<str, str>::const_iterator it = m.find(ext);
	str mime = (it != m.end()) ? it->second : "text/plain";

	return mime;
}

void redirResponse( Response& response, Location location ) {
	response.setStatus(location._redirCode);
	response.addHeaders("Location", location._redirTarget);
	response.setBody("Moved Permanently. Redirecting to " + location._redirTarget);
	response.addHeaders("Content-Length", iToString(response.getContentLength()));
}

void genResponse( Response& response, str& src, ServerEntry* _srvEntry ) {
	std::ifstream file(src.c_str());
	sstream buffer;
	if (file.is_open()) {
		buffer << file.rdbuf();
		response.setBody(buffer.str());
		response.setStatus(OK);
		response.addHeaders("Content-Length", iToString(response.getContentLength()));
		response.addHeaders("Content-Type", getContentType(src.substr(1)));
		file.close();
	} else {
		getSrvErrorPage(response, _srvEntry, NOT_FOUND);
	}
}

bool validateRequest( ServerEntry *_srvEntry, Request& request, Response& response, Location& location ) {
	if (request.getBody().length()) {
		if (request.getBody().length() > _srvEntry->_maxBodySize) {
			getSrvErrorPage(response, _srvEntry, CONTENET_TOO_LARGE);
			return false;
		}
	}
	else if (location._allowedMethods.find(request.getMethod())
		== location._allowedMethods.end()) {
		getSrvErrorPage(response, _srvEntry, METHOD_NOT_ALLOWED);
		return false;
	}
	else if (location._redirSet) {
		redirResponse(response, location);
		return false;
	}
	return true;
}

str getDateHeader( void ) {
	time_t now = time(0);
	struct tm _time;

	_time = *localtime(&now);
	char buf[100];

	strftime(buf, sizeof(buf), "%a, %d %b %Y %X ", &_time);

	return str(buf);
}

int fileStat( const str& src ) {
	struct stat st;

	if (lstat(src.c_str(), &st) != 0) {
		return -1;
	}
	if (S_ISDIR(st.st_mode)) return 0;
	if (S_ISREG(st.st_mode)) return 1;

	return -2;
}

bool isFileExist( str& src ) {
	if (access(src.c_str(), F_OK) == 0)
		return true;
	return false;
}

bool genErrorResponse( Response& response, str& src, int code ) {
	std::ifstream file(src.c_str());
	sstream buffer;
	if (file.is_open()) {
		buffer << file.rdbuf();
		response.setBody(buffer.str());
		response.setStatus(code);
		response.addHeaders("Content-Length", iToString(response.getContentLength()));
		response.addHeaders("Content-Type", getContentType(src.substr(1)));
		file.close();
		return true;
	} else
		return false;
}

void getSrvErrorPage( Response& response, ServerEntry* _srvEntry, int code ) {
	std::map<int, str> errorPgs = _srvEntry->_errorPages;
	if (!errorPgs.empty()) {
		if (errorPgs.find(code) != errorPgs.end()) {
			str src = "." + _srvEntry->_root + errorPgs.find(code)->second;
			if (!genErrorResponse(response, src, code))
				defErrorResponse(response, code);
		} else {
			defErrorResponse(response, code);
			return;
		}
	} else {
		defErrorResponse(response, code);
		return;
	}
}

