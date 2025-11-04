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
	int _contentLength;
	std::map<str, str> _headers;
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
	const int& getContentLength( void ) const;
	const std::map<str, str>& getHeaders( void ) const;

	void setStatus( int code );
	void addHeaders( const str& key, const str& value );
	void setBody( const str& bodyData );
	str generate( void ) const;
};

str iToString(int n);
// void notImplementedResponse( Response& response );
// void URItooLongResponse( Response& response );
// void BadRequestResponse( Response& response );
void deleteHandler( ServerEntry *_srvEntry, Request& request, Response& response );
void postHandler( ServerEntry *_srvEntry, Request& request, Response& response );
void getHandler( ServerEntry *_srvEntry, Request& request, Response& response );
void errorResponse( Response& response, int code);
bool startsWith( const str& path, const str& start );
Location getLocation( ServerEntry *_srvEntry, Request& request );
std::deque<str> splitPath( const str& path );

#endif