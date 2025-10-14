#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "./Utils.hpp"
#include "./TypeDefs.hpp"
#include <map>
#include <unordered_map>

class Request {
private:
	str _method;
	str _Uri;
	str _path;
	str _version;
	str _body;
	std::unordered_map<str, str> _queryParams;
	std::map<str, str> _headers;

	static const char* valid_methods[];

	bool is_valid_method( const str& method ) const;
	bool parse_query_params( const str& path );

public:
	Request( void );
	~Request();

	const str& getMethod( void ) const;
	const str& getreqTarget( void ) const;
	const str& getVersion( void ) const;
	const std::unordered_map<str, str>& getQueryParams( void ) const;
	const std::map<str, str>& getHeaders( void ) const;
	const str& getBody( void ) const;
	const str& getPath( void ) const;

	bool parseReqline( const char* input );
	void initHeaders( const char* input );
	void initBody( const char* input );
};

bool UriAllowedChars( str& uri );

#endif
