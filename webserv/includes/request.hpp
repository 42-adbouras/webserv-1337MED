#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "./Utils.hpp"
#include "./TypeDefs.hpp"
#include <sys/socket.h>
#include <map>
#include "./serverHeader/Client.hpp"

class Response;
class Client;

struct StatusEntry {
	int code;
	str message;
};

class Request {
private:
	str _method;
	str _Uri;
	str _path;
	str _version;
	str _body;
	std::map<str, str> _queryParams;
	std::map<str, str> _headers;

	static const char* valid_methods[];

	bool is_valid_method( const str& method ) const;
	bool parse_query_params( const str& path );

public:
	Request( void );
	Request( Client& clt );
	~Request();

	class RequestException : public std::exception {
	private:
		str msg;
	public:
		RequestException( const str& msg ) : msg(msg) { }
		virtual const char* what() const throw() {
			return msg.c_str();
		}
		virtual ~RequestException() throw() { }
	};

	const str& getMethod( void ) const;
	const str& getreqTarget( void ) const;
	const str& getVersion( void ) const;
	const std::map<str, str>& getQueryParams( void ) const;
	const std::map<str, str>& getHeaders( void ) const;
	const str& getBody( void ) const;
	const str& getPath( void ) const;

	bool parseReqline( const char* input, Response& response );
	void initHeaders( const char* input );
	void initBody( const char* input );
};

bool UriAllowedChars( str& uri );
void requestHandler( const char* buffer, int socket );

#endif