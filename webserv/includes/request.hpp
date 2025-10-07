#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include "Utils.hpp"
#include "TypeDefs.hpp"

class Request {
private:
	str _method;
	str _reqTarget;
	str _version;

public:
	Request( void );
	~Request();

	const str& getMethod( void ) const;
	const str& getreqTarget( void ) const;
	const str& getVersion( void ) const;

	bool parseReqline( const char* input );
};

#endif