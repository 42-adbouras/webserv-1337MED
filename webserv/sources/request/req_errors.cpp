#include "../../includes/request.hpp"
#include "../../includes/response.hpp"

bool headersCheck( Request& request ) {
	HeadersMap headers = request.getHeaders();
	HeadersMap::iterator it = headers.begin();
	while( it != headers.end() ) {
		if (it->second == "")
			return false;
		++it;
	}
	HeadersMap::iterator hostIt = headers.find("Host");
	if (hostIt == headers.end())
		return false;
	if (hostIt->second.empty())
		return false;
	if (request.getBody() != "") {
		HeadersMap::iterator clIt = headers.find("Content-Length");
		if (clIt == headers.end())
			return false;
		if (!isNumber(clIt->second))
			return false;
	}
	return true;
}

bool requestErrors( Request& request, Response& response, ServerEntry* _srvEntry ) {
	const HeadersMap& headers = request.getHeaders();
	HeadersMap::const_iterator te = headers.find("Transfer-Encoding");
	HeadersMap::const_iterator cl = headers.find("Content-Length");
	if (te != headers.end()) {
		if(te->second != "chunked") {
			getSrvErrorPage(response, _srvEntry, NOT_IMPLEMENTED);
			return false;
		}
	}
	if (te == headers.end() && cl == headers.end() && request.getMethod() == "POST") {
		getSrvErrorPage(response, _srvEntry, BAD_REQUEST);
		return false;
	}
	if (te != headers.end() && cl != headers.end()) {
		getSrvErrorPage(response, _srvEntry, BAD_REQUEST);
		return false;
	}
	if (!headersCheck(request))
		getSrvErrorPage(response, _srvEntry, BAD_REQUEST);
	return true;
}

void defErrorResponse( Response& response, int code) {
	response.setStatus(code);
	str f = "./www/defaultErrorPages/" + toString(code) + ".html";
	std::ifstream file(f.c_str());
	str errorExpt = toString(code) + " error default page not found!";
	if (file.is_open()) {
		sstream buffer;
		buffer << file.rdbuf();
		response.setBody(buffer.str());
		// response.addHeaders("Content-Length", toString(response.getContentLength()));
		response.addHeaders("Content-Type", getContentType(f.substr(1)));
		file.close();
	} else
		throw Request::RequestException(errorExpt);
}
