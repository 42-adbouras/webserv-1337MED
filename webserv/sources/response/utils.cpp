#include "../../includes/response.hpp"
#include "../../includes/request.hpp"
#include <algorithm>

static str toLower( const str& s ) {
	str out = s;
	std::transform(out.begin(), out.end(), out.begin(), static_cast<int(*)(int)>(std::tolower));
	return out;
}

static std::map<str, str> rMimeMape() {
	std::map<str, str> rM;
	if (rM.empty()) {
		rM["text/html"] = ".html";
		rM["text/css"] = ".css";
		rM["text/x-python"] = ".py";
		rM["application/x-httpd-php"] = ".php";
		rM["application/javascript"] = ".js";
		rM["application/json"] = ".json";
		rM["application/xml"] = ".xml";
		rM["text/plain"] = ".txt";
		rM["image/png"] = ".png";
		rM["image/jpeg"] = ".jpg";
		rM["image/gif"] = ".gif";
		rM["image/svg+xml"] = ".svg";
		rM["application/pdf"] = ".pdf";
		rM["application/zip"] = ".zip";
		rM["audio/mpeg"] = ".mp3";
		rM["video/mp4"] = ".mp4";
		rM["video/x-matroska"] = ".mkv";
		rM["application/font-woff"] = ".woff";
		rM["application/font-woff2"] = ".woff2";
	}
	return rM;
}

static std::map<str, str> mimeMap() {
	std::map<str, str> m;
	if (m.empty()) {
		m[".html"] = "text/html; charset=UTF-8";
		m[".htm"]  = "text/html; charset=UTF-8";
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

str getFileType( const str& type ) {
	const std::map<str, str>& rM = rMimeMape();
	std::map<str, str>::const_iterator it = rM.find(type);
	str rMime = (it != rM.end()) ? it->second : ".txt";

	return rMime;
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

void redirectionResponse( Response& response, Location location ) {
	response.setStatus(location._redirCode);
	response.addHeaders("Location", location._redirTarget);
	response.setBody("Moved Permanently. Redirecting to " + location._redirTarget);
	// response.addHeaders("Content-Length", toString(response.getContentLength()));
}

void genResponse( Response& response, str& src, ServerEntry* _srvEntry ) {
	struct stat st;

	if (stat(src.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
		getSrvErrorPage(response, _srvEntry, NOT_FOUND);
		return;
	}

	response.setStatus(OK);
	response.addHeaders("Accept-Ranges", "bytes");
	response.addHeaders("Content-Type", getContentType(src));
	response.addHeaders("Content-Length", toString((size_t)st.st_size));

	response._streamFile = true;
	response._filePath = src;
	response._fileSize = st.st_size;
	response._bytesSent = 0;
	response._fileOffset = 0;
}

bool validateRequest( ServerEntry *_srvEntry, Request& request, Response& response, Location& location ) {
	if (request.getBody().length()) {
		if (request.getBody().length() > _srvEntry->_maxBodySize) {
			getSrvErrorPage(response, _srvEntry, CONTENT_TOO_LARGE);
			return false;
		}
	}
	else if (location._allowedMethods.find(request.getMethod())
		== location._allowedMethods.end()) {
		getSrvErrorPage(response, _srvEntry, METHOD_NOT_ALLOWED);
		return false;
	}
	else if (location._redirSet) {
		redirectionResponse(response, location);
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

void HtmlDefaultErrorsPages( Response& response, int code ) {
	std::map<int, str> statusMap = getStatusMap();
	std::map<int, str>::const_iterator it = statusMap.find(code);
	str statusText;
	if ( it != statusMap.end() ) {
		statusText = it->second;
	}
	std::ostringstream html;
	html << "<!DOCTYPE html>\n"
		 << "<html>\n"
		 << "<head><title>Index of " << code << "</title></head>\n"
		 << "<body>\n"
		 << "<h1>" << code << " " << statusText << "</h1>\n"
		 << "</body>\n";
	
	response.setBody(html.str());
	response.addHeaders("Content-Type", "text/html; charset=UTF-8");
}

void defErrorResponse( Response& response, int code) {
	response.setStatus(code);
	str f = "./www/defaultErrorPages/" + toString(code) + ".html";
	std::ifstream file(f.c_str());
	str errorExpt = toString(code) + " error default page not found!";
	if (file.is_open()) {
		sstream buffer;
		buffer << file.rdbuf();
		response.setStatus(code);
		response.setBody(buffer.str());
		// response.addHeaders("Content-Length", toString(response.getContentLength()));
		response.addHeaders("Content-Type", getContentType(f.substr(1)));
		file.close();
	} else
		HtmlDefaultErrorsPages( response, code );
}

bool genErrorResponse( Response& response, str& src, int code ) {
	std::ifstream file(src.c_str());
	sstream buffer;
	if (file.is_open()) {
		buffer << file.rdbuf();
		response.setBody(buffer.str());
		response.setStatus(code);
		// response.addHeaders("Content-Length", toString(response.getContentLength()));
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

size_t sToSize_t( const str& str ) {
	sstream ss(str);

	size_t value = 0;
	ss >> value;

	return value;
}

long long getFileSize( const str& src ) {
	struct stat st;

	if (stat(src.c_str(), &st) == -1)
		return -1;
	if (!S_ISREG(st.st_mode))
		return -1;

	return (long long)st.st_size;
}
