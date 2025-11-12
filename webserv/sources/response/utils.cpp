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
		m[".html"] = "text/html";
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
	str mime = (it != m.end()) ? it->second : "application/octet-stream";

	return mime;
}

void redirResponse( Response& response, Location location ) {
	response.setStatus(location._redirCode);
	response.addHeaders("Location", location._redirTarget);
	response.setBody("Moved Permanently. Redirecting to " + location._redirTarget);
	response.addHeaders("Content-Length", iToString(response.getContentLength()));
}

void genResponse( Response& response, str& src ) {
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
		errorResponse(response, NOT_FOUND);
	}
}

bool validateRequest( ServerEntry *_srvEntry, Request& request, Response& response, Location& location ) {
	if (request.getBody().length()) {
		if (request.getBody().length() > _srvEntry->_maxBodySize) {
			errorResponse(response, CONTENET_TOO_LARGE);
			return false;
		}
	}
	else if (location._allowedMethods.find(request.getMethod())
		== location._allowedMethods.end()) {
		errorResponse(response, METHOD_NOT_ALLOWED);
		return false;
	}
	else if (location._redirSet) {
		redirResponse(response, location);
		return false;
	}
	return true;
}
