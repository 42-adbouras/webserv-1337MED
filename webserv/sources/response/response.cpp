#include "../../includes/response.hpp"

Response::Response( void )
	: _statusCode(0)
	, _statusText()
	, _version("HTTP/1.1")
	, _body()
	, _contentLength(0)
	, _headers() { }

Response::~Response() { }

const int& Response::getStatusCode( void ) const { return _statusCode; }
const str& Response::getStatusText( void ) const { return _statusText; }
const str& Response::getVersion( void ) const { return _version; }
const str& Response::getBody( void ) const { return _body; }
const int& Response::getContentLength( void ) const { return _contentLength; }
const std::unordered_map<str, str>& Response::getHeaders( void ) const { return _headers; }

void Response::setStatus( int code ) {
	switch (code) {
		case 200:
			_statusCode = 200;
			_statusText = "OK";
			break;
		case 201:
			_statusCode = 201;
			_statusText = "Created";
			break;
		case 204:
			_statusCode = 204;
			_statusText = "No Content";
			break;
		case 301:
			_statusCode = 301;
			_statusText = "Moved Permanently";
			break;
		case 400:
			_statusCode = 400;
			_statusText = "Bad Request";
			break;
		case 403:
			_statusCode = 403;
			_statusText = "Forbidden";
			break;
		case 404:
			_statusCode = 404;
			_statusText = "Not Found";
			break;
		case 405:
			_statusCode = 405;
			_statusText = "Method Not Allowd";
			break;
		case 409:
			_statusCode = 409;
			_statusText = "Conflict";
			break;
		case 413:
			_statusCode = 413;
			_statusText = "Content Too Large";
			break;
		case 414:
			_statusCode = 414;
			_statusText = "URI Too Long";
			break;
		case 500:
			_statusCode = 500;
			_statusText = "Internal Server Error";
			break;
		case 501:
			_statusCode = 501;
			_statusText = "Not Implemented";
			break;
	}
}

/* void Response::addHeaders( const str& key, const str& value ) {
	
} */

void Response::setBody( const str& bodyData ) {
	_body = bodyData;
	_contentLength = _body.length();
}

str Response::generate( void ) const {
	sstream ss;
	sstream s;
	ss << _statusCode;

	str HTTPresponse = _version + " " + ss.str() + " " + _statusText + BREAK_LINE;
	HTTPresponse += "Host: localohost:8080\r\n";
	HTTPresponse += "Content-Type: text/html\r\n";
	s << _contentLength;
	HTTPresponse += "Content-Length: " + s.str() + BREAK_LINE;
	HTTPresponse += BREAK_LINE + _body;

	return HTTPresponse;
}