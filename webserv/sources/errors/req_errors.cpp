#include "../../includes/request.hpp"
#include "../../includes/response.hpp"

bool headersCheck( Request& request ) {
	std::map<str, str> headers = request.getHeaders();
	std::map<str, str>::iterator it = headers.begin();
	while( it != headers.end() ) {
		if (it->second == "")
			return false;
		++it;
	}
	std::map<str, str>::iterator hostIt = headers.find("Host");
	if (hostIt == headers.end())
		return false;
	if (hostIt->second.empty())
		return false;
	if (request.getBody() != "") {
		std::map<str, str>::iterator clIt = headers.find("Content-Length");
		if (clIt == headers.end())
			return false;
		if (!isNumber(clIt->second))
			return false;
	}
	return true;
}

bool requestErrors( Request& request, Response& response ) {
	const std::map<str, str>& headers = request.getHeaders();
	std::map<str, str>::const_iterator te = headers.find("Transfer-Encoding");
	std::map<str, str>::const_iterator cl = headers.find("Content-Length");
	if (te != headers.end()) {
		if(te->second != "chunked") {
			errorResponse(response, NOT_IMPLEMENTED);
			return false;
		}
	}
	if (te == headers.end() && cl == headers.end() && request.getMethod() == "POST") {
		errorResponse(response, BAD_REQUEST);
		return false;
	}
	if (!headersCheck(request))
		errorResponse(response, BAD_REQUEST);
	return true;
}

void errorResponse( Response& response, int code) {
	response.setClientState(code);
	str f = "./www/defaultErrorPages/" + iToString(code) + ".html";
	std::ifstream file(f.c_str());
	str errorExpt = iToString(code) + " error default page not found!";
	if (!file.is_open()) {
		throw Request::RequestException(errorExpt);
	} else {
		sstream buffer;
		buffer << file.rdbuf();
		response.setBody(buffer.str());
		response.addHeaders("Content-Length", iToString(response.getContentLength()));
	}
}
