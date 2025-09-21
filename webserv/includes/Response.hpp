#pragma once

#include <string>
#include <map>
#include <fstream> // IWYU pragma: keep
#include <sstream> // IWYU pragma: keep

typedef std::string str;

class Response
{
private:
	int					_status;
	str					_reason;
	std::map<str, str>	_headers;
	str					_body;
public:
	Response( void );

	int			getStatus( void ) const;
	const str&	getReason( void ) const;
	const str&	getBody( void )   const;
	str&		getBody( void );

	void		setStatus( int code, const str &reason = str() );
	void		setHeader( const str &k, const str &v );
	void		setBody( const str &b );
	void		ensureHeader( const str &k, const str &v );
	void		finalize( bool keepAlive = true );
	str			toString( void ) const;

	static str		getContentType( const str &path );
	static Response	fromFile( const str &reqPath, const str &root );
	static Response	methodNotAllowed( void );

private:
	static str		defaultReason( int code );
};
