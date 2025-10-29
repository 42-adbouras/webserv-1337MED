#include "../../includes/request.hpp"
#include "../../includes/response.hpp"
#include "Client.hpp"

Request::Request( void )
	: _method()
	, _Uri()
	, _path()
	, _version()
	, _body()
	, _queryParams()
	, _headers() { }

Request::~Request() { }

Request& Request::operator=( const Request& req ) {
	if (this != &req) {
		this->_method = req._method;
		this->_Uri = req._Uri;
		this->_path = req._path;
		this->_version = req._version;
		this->_body = req._body;
		this->_queryParams = req._queryParams;
		this->_headers = req._headers;
		this->_buffer = req._buffer;
	}
	return *this;
}

const str& Request::getMethod( void ) const { return _method; }
const str& Request::getreqTarget( void ) const { return _Uri; }
const str& Request::getVersion( void ) const { return _version; }
const std::map<str, str>& Request::getQueryParams( void ) const { return _queryParams; }
const std::map<str, str>& Request::getHeaders( void ) const { return _headers; }
const str& Request::getBody( void ) const { return _body; }
const str& Request::getPath( void ) const { return _path; }
const str& Request::getBuffer( void ) const { return _buffer; }
void Request::setBuffer( char* buffer ) {
	_buffer = buffer;
}

const char* Request::valid_methods[] = {
	"GET", "POST", "DELETE", 0
};
bool Request::is_valid_method( const str& method ) const {
	for (size_t i=0; valid_methods[i]!=0; ++i) {
		if (method == valid_methods[i])
			return true;
	}
	return false;
}

bool Request::parse_query_params( const str& path ) {
	_path = path;

	str::size_type pos = path.find('?');
	if (pos == str::npos)
		return true;
	
	_path = path.substr(0, pos);
	str query = path.substr(pos + 1);

	str param;
	sstream iss(query);
	while( std::getline(iss, param, '&')) {
		str::size_type eq_pos = param.find('=');
		if (eq_pos != str::npos) {
			str key = param.substr(0, eq_pos);
			str value = param.substr(eq_pos + 1);
			_queryParams[key] = value;
		} else
			return false;
	}
	return true;
}

bool Request::parseReqline( const char* input, Response& response ) {
	str raw = str(input);

	sstream stream(raw);
	if(!(stream >> _method >> _Uri >> _version)) {
		response.setStatus(400);
		return false;
	}
	if(!is_valid_method( _method )) {
		notImplementedResponse(response);
		return false;
	}
	if(_version != "HTTP/1.1") {
		response.setStatus(400);
		return false;
	}
	if (_Uri.length() > 2048) {
		URItooLongResponse(response);
		return false;
	}
	if(!parse_query_params( _Uri ) || !UriAllowedChars( _Uri )) {
		response.setStatus(400);
		return false;
	}

	return true;
}

static str trim( const str& s ) {
	size_t start = s.find_first_not_of(" \t");
	size_t end = s.find_last_not_of(" \t");
	return (start == str::npos) ? "" : s.substr(start, end - start + 1);
}

void Request::initHeaders( const char* input ) {
	str raw(input);
	sstream stream(raw);
	str line;

	std::getline(stream, line);

	while(std::getline(stream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		
		str::size_type colon_pos = line.find(':');
		if (colon_pos != str::npos) {
			str key = trim(line.substr(0, colon_pos));
			str value = trim(line.substr(colon_pos + 1));
			_headers[key] = value;
		}
	}
}

void Request::initBody( const char* input ) {
	str raw(input);
	str::size_type pos = raw.find("\r\n\r\n");

	if (pos != str::npos)
		_body = raw.substr(pos + 4);
}

void requestHandler( Client& client ) {
	Request request;

	char buffer[3000];
	ssize_t rByte;
	if((rByte = recv(client.getFd(), buffer, sizeof(buffer), 0)) > 0) {
		buffer[rByte] = '\0';
		client.setStatus(KEEP_ALIVE);
	}
	if(rByte == 0)
		client.setStatus(DISCONNECT);
	else if (rByte < 0)
		std::cerr << "recv set errno to: " << strerror(errno) << std::endl;

	request.setBuffer( buffer );
	request.initHeaders( buffer );
	request.initBody( buffer );
	client.setRequest(request);
}

void sendResponse( Client& client ) {
	Response response;
	Request request = client.getRequest();
	if (!request.parseReqline( request.getBuffer().c_str(), response )) {
		str content = response.generate();
		send(client.getFd(), content.c_str(), content.length(), 0);
		return;
	} else {
		std::ifstream file("./www/index.html");
		if (!file.is_open())
			response.setStatus(404);
		else {
			response.setStatus(200);
			sstream buff;
			buff << file.rdbuf();
			str content = buff.str();
			response.setBody(content);
			response.addHeaders("Host", "localhost:8080");
			response.addHeaders("Content-Type", "text/html");
			response.addHeaders("Content-Length", "230");
		}
	}
	str content = response.generate();
	send(client.getFd(), content.c_str(), content.length(), 0);
}