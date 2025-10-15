#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__

#include "./Utils.hpp"
#include "./TypeDefs.hpp"
#include "./request.hpp"

#define BREAK_LINE "\r\n"

class Response {
private:
	int _statusCode;
	str _statusText;
	str _version;
	str _body;
	int _contentLength;
	std::map<str, str> _headers;
public:
	Response( void );
	~Response();

	const int& getStatusCode( void ) const;
	const str& getStatusText( void ) const;
	const str& getVersion( void ) const;
	const str& getBody( void ) const;
	const int& getContentLength( void ) const;
	const std::map<str, str>& getHeaders( void ) const;

	void setStatus( int code );
	void addHeaders( const str& key, const str& value );
	void setBody( const str& bodyData );
	str generate( void ) const;
};

#endif