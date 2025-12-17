#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "./Utils.hpp"
#include "./TypeDefs.hpp"
#include "./response.hpp"
// #include "./serverHeader/CookiesSessionManager.hpp" /*---------*/
#include <sys/socket.h>
#include <deque>

class Client;
class Response;
class CookiesSessionManager;

class Request {
private:
	str _method;
	str _Uri;
	str _path;
	str _version;
	str _body;
	std::vector<char> _buffer;
	str _location;
	QueryMap _queryParams;
	HeadersMap _headers;
	ServerEntry *_srvEntry;

	static const char* valid_methods[];

	bool is_valid_method( const str& method ) const;
	bool parse_query_params( const str& path );

public:
	Request( void );
	~Request();
	Request& operator=( const Request& req );

	class RequestException : public std::exception {
	private:
		str msg;
	public:
		RequestException( const str& message ) : msg(message) { }
		virtual ~RequestException() throw() { }

		virtual const char* what() const throw() {
			return msg.c_str();
		}
	};

	const str& getMethod( void ) const;
	const str& getreqTarget( void ) const;
	const str& getVersion( void ) const;
	const QueryMap& getQueryParams( void ) const;
	const HeadersMap& getHeaders( void ) const;
	const str& getBody( void ) const;
	const str& getPath( void ) const;
	const std::vector<char>& getBuffer( void ) const;
	const str& getUri( void ) const;
	void setBuffer( std::vector<char> buffer );
	void setLocation( str& location );
	void setPath( const str& path );
	const str& getLocation( void ) const;
	ServerEntry* getSrvEntry( void ) const;
	void setSrvEntry( ServerEntry* srvEnt );

	bool requestLineErrors( Response& response, ServerEntry* _srvEntry );
	void parseRequestLine( str& input );
	void initHeaders( str& input );
	void setBody( str& body );
};

bool UriAllowedChars( str& uri );
void requestHandler( Client& client );
void sendResponse( Client& client, CookiesSessionManager& sessionManager ); /*---------*/
str normalizePath( const str& path );
str getHost( const HeadersMap& headers );
str getSource( Request& request, ServerEntry* _srvEntry, Response& response );
bool requestErrors( Request& request, Response& response, ServerEntry* _srvEntry );
bool isNumber(str& s);
std::deque<str> splitPath( const str& path );
Location getLocation( ServerEntry *_srvEntry, Request& request, Response& response );
str urlDecode( const str& path );

#endif
