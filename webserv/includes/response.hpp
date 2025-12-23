#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__

#include "./TypeDefs.hpp"
#include "../sources/request/utils.tpp"
#include "./serverHeader/SocketManager.hpp"

#define OK 200
#define CREATED 201
#define NO_CONTENT 204
#define PARTIAL_CONTENT 206
#define MOVED_PERMANENTLY 301
#define BAD_REQUEST 400
#define FORBIDDEN 403
#define NOT_FOUND 404
#define METHOD_NOT_ALLOWED 405
#define CONFLICT 409
#define CONTENT_TOO_LARGE 413
#define URI_TOO_LONG 414
#define RANGE_NOT_SATISFIABLE 416
#define INTERNAL_SERVER_ERROR 500
#define NOT_IMPLEMENTED 501
#define GATEWAY_TIMEOUT 504
#define HTTP_VERSION_NOT_SUPPORTED 505

class Request;

struct StatusEntry {
	int code;
	str message;
};

struct Range {
	long long start;
	long long end;
	bool valid;
	str error;
};

class Response {
private:
	int _statusCode;
	str _statusText;
	str _version;
	str _body;
	size_t _contentLength;
	HeadersMap _headers;
	str _source;
	bool flag;
public:
	Response( void );
	~Response();
	Response& operator=( const Response& res );

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

	bool _streamFile;
	str _filePath;
	off_t _fileOffset;
	off_t _fileSize;
	off_t _bytesSent;

	const int& getStatusCode( void ) const;
	const str& getStatusText( void ) const;
	const str& getVersion( void ) const;
	const str& getBody( void ) const;
	str& getSrc( void );
	bool getFlag( void ) const;
	const size_t& getContentLength( void ) const;
	HeadersMap& getHeaders( void );

	void setStatus( int code );
	void addHeaders( const str& key, const str& value );
	void setBody( const str& bodyData );
	void setSrc( const str& source );
	void setFlag( bool flg );
	str generate( void );
};

str iToString(size_t n);
void deleteHandler( ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client );
void postHandler( ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client );
void getHandler( ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client );
void defErrorResponse( Response& response, int code);
bool startsWith( const str& path, const str& start );
Location getLocation( Request& request, Response& response );
void redirectionResponse( Response& response, Location location );
str getContentType( const str& path );
void genResponse( Response& response, str& src, ServerEntry* _srvEntry );
bool validateRequest( ServerEntry *_srvEntry, Request& request, Response& response, Location& location );
str getDateHeader( void );
int fileStat( const str& src );
bool isFileExist( str& src );
bool isCgi( Location& location, str& src, Client& client, Request& request );
void getSrvErrorPage( Response& response, ServerEntry* _srvEntry, int code );
size_t sToSize_t( const str& str );
str getFileType( const str& type );
long long getFileSize( const str& src );
Range parseRangeHeader(const std::string& rangeHeader, long long fileSize);
ServerEntry* getSrvBlock( serverBlockHint& _srvBlockHint, Request& request);
const std::map<int, str>& getStatusMap();

#endif