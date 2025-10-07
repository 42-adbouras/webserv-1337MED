#include "../../includes/request.hpp"

Request::Request( void )
	: _method()
	, _reqTarget()
	, _version() { }

Request::~Request() { }

const str& Request::getMethod( void ) const { return _method; }
const str& Request::getreqTarget( void ) const { return _reqTarget; }
const str& Request::getVersion( void ) const { return _version; }

bool Request::parseReqline( const char* input ) {
	str raw = str(input);

	std::stringstream stream(raw);
	if(!(stream >> _method >> _reqTarget >> _version))
		return false;
	return true;
}
