#include "../../includes/request.hpp"

Request::Request( void )
	: _method()
	, _Uri()
	, _path()
	, _version()
	, _body()
	, _queryParams()
	, _headers() { }

Request::~Request() { }

const str& Request::getMethod( void ) const { return _method; }
const str& Request::getreqTarget( void ) const { return _Uri; }
const str& Request::getVersion( void ) const { return _version; }
const std::map<str, str>& Request::getQueryParams( void ) const { return _queryParams; }
const std::map<str, str>& Request::getHeaders( void ) const { return _headers; }
const str& Request::getBody( void ) const { return _body; }
const str& Request::getPath( void ) const { return _path; }

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
	std::istringstream iss(query);
	while( std::getline(iss, param, '&')) {
		str::size_type eq_pos = param.find('=');
		if (eq_pos != str::npos) {
			str key = param.substr(0, eq_pos);
			str value = param.substr(eq_pos + 1);
			_queryParams[key] = value;
		}
	}
	return true;
}

bool Request::parseReqline( const char* input ) {
	str raw = str(input);

	sstream stream(raw);
	if(!(stream >> _method >> _Uri >> _version))
		return false;
	if(!is_valid_method( _method ))
		return false;
	if(_version != "HTTP/1.1")
		return false;
	if(!parse_query_params( _Uri ) || !UriAllowedChars( _Uri )
		|| _Uri.length() > 2048)
		return false;

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
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		
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
