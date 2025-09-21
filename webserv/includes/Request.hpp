#pragma once

#include <string>
#include <sstream> // IWYU pragma: keep

typedef std::string str;

class Request
{
private:
	str 	_method;
	str 	_path;
	str 	_version;
public:
	Request( void );
	bool		parse( const str& raw );

	const str&	getMethod( void )  const;
	const str&	getVersion( void ) const;
	const str&	getPath( void )    const;

	str&		getPath( void );
};
