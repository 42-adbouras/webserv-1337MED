#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__

#include "./Utils.hpp"
#include "./TypeDefs.hpp"
#include "SocketManager.hpp"

#define OK 200
#define CREATED 201
#define NO_CONTENT 204
#define MOVED_PERMANENTLY 301
#define BAD_REQUEST 400
#define FORBIDDEN 403
#define NOT_FOUND 404
#define METHOD_NOT_ALLOWED 405
#define CONFLICT 409
#define CONTENET_TOO_LARGE 413
#define URI_TOO_LONG 414
#define INTERNAL_SERVER_ERROR 500
#define NOT_IMPLEMENTED 501
#define HTTP_VERSION_NOT_SUPPORTED 505

class Request;

struct StatusEntry {
	int code;
	str message;
};

class Response {
private:
	int _statusCode;
	str _statusText;
	str _version;
	str _body;
	size_t _contentLength;
	HeadersMap _headers;
public:
	Response( void );
	~Response();

	class ResponseException : public std::exception {
	private:
		str msg;
	public:
		ResponseException( const str& message ) : msg(message) { }
		virtual ~ResponseException() throw() { }

		virtual const char* what() const throw() {
			return msg.c_str();
		}
	};

	const int& getStatusCode( void ) const;
	const str& getStatusText( void ) const;
	const str& getVersion( void ) const;
	const str& getBody( void ) const;
	const size_t& getContentLength( void ) const;
	const HeadersMap& getHeaders( void ) const;

	void setStatus( int code );
	void addHeaders( const str& key, const str& value );
	void setBody( const str& bodyData );
	str generate( void ) const;
};

str iToString(size_t n);
void deleteHandler( ServerEntry *_srvEntry, Request& request, Response& response, str& src );
void postHandler( ServerEntry *_srvEntry, Request& request, Response& response, str& src );
void getHandler( ServerEntry *_srvEntry, Request& request, Response& response, str& src );
void errorResponse( Response& response, int code);
bool startsWith( const str& path, const str& start );
Location getLocation( ServerEntry *_srvEntry, Request& request, Response& response );
void redirResponse( Response& response, Location location );
str getContentType( const str& path );
void genResponse( Response& response, str& src );
bool validateRequest( ServerEntry *_srvEntry, Request& request, Response& response, Location& location );
str getDateHeader( void );
int fileStat( const str& src );
bool isFileExist( str& src );

#endif