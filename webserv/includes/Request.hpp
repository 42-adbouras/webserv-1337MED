#pragma once

#include <string>
#include <sstream> // IWYU pragma: keep
#include <map>

typedef std::string str;

class Request
{
private:
	str 	_method;
	str 	_path;
	str 	_version;
	str		_body;
	bool	_cgi;
	str		_cgiPath;
	str		_ntrp;
	std::map<str,str> _headers;
public:
	Request( void );
	bool		parse( const str& raw );

	const str&	getMethod( void )  const;
	const str&	getVersion( void ) const;
	const str&	getPath( void )    const;
	const str&	getBody( void )    const;
	const str&	getNTRP( void )    const { return (_ntrp); }
	const str&	getCGIPath( void )    const { return (_cgiPath); }
	void		setCGI( str ntrp, str path ) {
		_cgi = true;
		_cgiPath = path;
		_ntrp = ntrp;
	};

	str&		getPath( void );
};
