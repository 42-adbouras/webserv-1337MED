#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "./Utils.hpp" // IWYU pragma: keep
#include "./TypeDefs.hpp"
#include "response.hpp"
#include <sys/socket.h>
// #include <map>

class Client;
class Response;
class CookiesSessionManager;

// enum ParseState {
// 	PARSING_HEADERS,
// 	PARSING_BODY,
// 	REQUEST_COMPLETE
// };

class Request {
private:
	str _method;
	str _Uri;
	str _path;
	str _version;
	str _body;
	std::vector<char> _buffer;
	str _location;
	// str _leftover;
	// size_t _expectedBodyLength;
	// bool _isChunked;
	QueryMap _queryParams;
	HeadersMap _headers;

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

	// ParseState _state;

	const str& getMethod( void ) const;
	const str& getreqTarget( void ) const;
	const str& getVersion( void ) const;
	const QueryMap& getQueryParams( void ) const;
	const HeadersMap& getHeaders( void ) const;
	const str& getBody( void ) const;
	const str& getPath( void ) const;
	const std::vector<char>& getBuffer( void ) const;
	const str& getUri( void ) const;
	const str& getLeftover( void ) const;
	size_t getExpectedBodyLength( void ) const;
	bool getIsChunked( void ) const;
	void setBuffer( std::vector<char> buffer );
	void setLocation( str& location );
	void setPath( const str& path );
	// void setExpectedBodyLength( size_t lgth );
	// void setIsChunked( bool chunked );
	// void setLeftover( str& leftover );
	const str& getLocation( void ) const;

	bool parseReqline( str& input, Response& response, ServerEntry* _srvEntry );
	void parseRequestLine( str& input );
	void initHeaders( str& input );
	void initBody( str& input );
};

bool UriAllowedChars( str& uri );
void requestHandler( Client& client );
void sendResponse( Client& client );
str normalizePath( const str& path );
str getHost( const HeadersMap& headers );
str getSource( Request& request, ServerEntry* _srvEntry, Response& response );
bool requestErrors( Request& request, Response& response, ServerEntry* _srvEntry );
bool isNumber(str& s);
std::deque<str> splitPath( const str& path );
Location getLocation( ServerEntry *_srvEntry, Request& request, Response& response );
str urlDecode( const str& path );

#endif
