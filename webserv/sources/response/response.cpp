#include "../../includes/response.hpp"

Response::Response( void )
	: _statusCode(0)
	, _statusText()
	, _version("HTTP/1.1")
	, _body()
	, _contentLength(0) {

		_headers["Server"] = "webServer";
		_headers["Content-Type"] = "text/plain";
		_headers["Content-Length"] = "0";
		_headers["Connection"] = "keep-alive";
}

Response::~Response() { }

const int& Response::getStatusCode( void ) const { return _statusCode; }
const str& Response::getStatusText( void ) const { return _statusText; }
const str& Response::getVersion( void ) const { return _version; }
const str& Response::getBody( void ) const { return _body; }
const int& Response::getContentLength( void ) const { return _contentLength; }
const std::map<str, str>& Response::getHeaders( void ) const { return _headers; }

static const StatusEntry StatusEntries[] = {
	{200, "OK"},
	{201, "Created"},
	{204, "No Content"},
	{301, "Moved Permanently"},
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{409, "Conflict"},
	{413, "Content Too Large"},
	{414, "URI Too Long"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"}
};

static const std::map<int, str>& getStatusMap() {
	static std::map<int, str> StatusMap;
	if (StatusMap.empty()) {
		for (size_t i=0; i<sizeof(StatusEntries)/sizeof(StatusEntry); ++i)
			StatusMap[StatusEntries[i].code] = StatusEntries[i].message;
	}
	return StatusMap;
}

std::map<int, str> statusMap = getStatusMap();

void Response::setStatus( int code ) {
	std::map<int, str>::const_iterator it = statusMap.find(code);
	if ( it != statusMap.end() ) {
		_statusCode = code;
		_statusText = it->second;
	}
}

void Response::addHeaders( const str& key, const str& value ) {
	_headers[key] = value;	
}

void Response::setBody( const str& bodyData ) {
	_body = bodyData;
	_contentLength = _body.length();
}

str Response::generate( void ) const {
	sstream ss;
	ss << _statusCode;

	str HTTPresponse = _version + " " + ss.str() + " " + _statusText + "\r\n";
	std::map<str, str>::const_iterator it = _headers.begin();
	while( it != _headers.end() ) {
		HTTPresponse += it->first + ": " + it->second + "\r\n";
		++it; 
	}
	HTTPresponse += "\r\n" + _body;

	return HTTPresponse;
}
