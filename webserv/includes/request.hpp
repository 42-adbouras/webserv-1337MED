#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "./Utils.hpp" // IWYU pragma: keep
#include "./TypeDefs.hpp"
#include <sys/socket.h>
#include <map>

class Client;
class Response;

class Request {
private:
	str _method;
	str _Uri;
	str _path;
	str _version;
	str _body;
	str _buffer;
	std::map<str, str> _queryParams;
	std::map<str, str> _headers;

	static const char* valid_methods[];

	bool is_valid_method( const str& method ) const;
	bool parse_query_params( const str& path );

public:
	Request( void );
	~Request();
	Request& operator=( const Request& req );

	const str& getMethod( void ) const;
	const str& getreqTarget( void ) const;
	const str& getVersion( void ) const;
	const std::map<str, str>& getQueryParams( void ) const;
	const std::map<str, str>& getHeaders( void ) const;
	const str& getBody( void ) const;
	const str& getPath( void ) const;
	const str& getBuffer( void ) const;
	void setBuffer( char* buffer );

	bool parseReqline( const char* input, Response& response );
	void initHeaders( const char* input );
	void initBody( const char* input );
};

bool UriAllowedChars( str& uri );
void requestHandler( Client& client );
void sendResponse( Client& client );

#endif