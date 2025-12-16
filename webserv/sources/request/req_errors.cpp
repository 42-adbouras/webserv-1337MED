#include "../../includes/request.hpp"
#include "../../includes/response.hpp"

bool headersCheck( Request& request ) {
	const HeadersMap& headers = request.getHeaders();
	HeadersMap::const_iterator it = headers.begin();
	while( it != headers.end() ) {
		if (it->second == "")
			return false;
		++it;
	}
	HeadersMap::const_iterator hostIt = headers.find("Host");
	if (hostIt == headers.end())
		return false;
	if (hostIt->second.empty())
		return false;
	if (request.getBody() != "") {
		HeadersMap::const_iterator clIt = headers.find("Content-Length");
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
