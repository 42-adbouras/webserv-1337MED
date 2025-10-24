#include "../includes/Request.hpp"

Request::Request()
	: _method()
	, _path()
	, _version() {}

bool	Request::parse( const str& raw )
{
		std::istringstream iss(raw);
		if (!(iss >> _method >> _path >> _version))
			return (false);
		size_t p = raw.find("\n\n");
		_body = raw.substr(p + 2);
		
		return (true);
}

const str& Request::getMethod()  const { return (this->_method); }
const str& Request::getVersion() const { return (this->_version); }
const str& Request::getPath() const { return (this->_path); }
const str& Request::getBody( void ) const { return (this->_body); };

str& Request::getPath( void ) { return (this->_path); }
