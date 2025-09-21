#include "../includes/Response.hpp"

Response::Response( void )
	: _status(200)
	, _reason("OK")
	, _headers()
	, _body() {}

int			Response::getStatus( void ) const { return (this->_status); }
const str&	Response::getReason( void ) const { return (this->_reason); }
const str&	Response::getBody( void )   const { return (this->_body); }
str&		Response::getBody( void ) { return (this->_body); }

void Response::setStatus( int code, const str& reason )
{
	this->_status = code;
	this->_reason = reason.empty() ? defaultReason(code) : reason;
}

void	Response::setHeader( const str &k, const str &v )
{
	this->_headers[k] = v;
}

void	Response::setBody( const str &b )
{
	this->_body = b;
}

void	Response::ensureHeader( const str &k, const str &v )
{
	if (this->_headers.find(k) == this->_headers.end())
		this->_headers[k] = v;
}

void	Response::finalize( bool keepAlive )
{
	std::ostringstream	oss;

	oss << this->_body.size();
	this->_headers["Content-Length"] = oss.str();
	ensureHeader("Connection", keepAlive ? "keep-alive" : "close");
}

str		Response::toString( void ) const
{
	std::ostringstream	out;

	out << "HTTP/1.1 " << this->_status << ' ' << this->_reason << "\r\n";
	for (std::map<str,str>::const_iterator it = this->_headers.begin(); it != this->_headers.end(); ++it)
		out << it->first << ": " << it->second << "\r\n";
	out << "\r\n" << _body;

	return (out.str());
}

str		Response::getContentType( const str &path )
{
	if (path.find(".html") != str::npos) return ("text/html");
	if (path.find(".css")  != str::npos) return ("text/css");
	if (path.find(".js")   != str::npos) return ("application/javascript");
	if (path.find(".png")  != str::npos) return ("image/png");
	if (path.find(".jpg")  != str::npos || path.find(".jpeg") != str::npos) return ("image/jpeg");
	return ("text/plain");
}

Response	Response::fromFile( const str& reqPath, const str& root )
{
	Response	resp;
	str			path = reqPath;
	str			absolute = root + path;

	if (!absolute.empty() && absolute[absolute.size()-1] == '/')
		absolute += "index.html";

	std::ifstream file(absolute.c_str());

	if (!file.is_open())
	{
		resp.setStatus(404);
		resp._body = "<h1>404 Not Found</h1>";
		resp.setHeader("Content-Type","text/html");
		resp.finalize(true);
		return (resp);
	}

	std::ostringstream	content;

	content << file.rdbuf();
	resp._body = content.str();
	resp.setHeader("Content-Type", getContentType(absolute));
	resp.setStatus(200);
	resp.finalize(true);
	return (resp);
}

Response	Response::methodNotAllowed()
{
	Response	r;

	r.setStatus(405);
	r._body = "<h1>405 Method Not Allowed</h1>";
	r.setHeader("Content-Type","text/html");
	r.finalize(true);
	return (r);
}

str			Response::defaultReason( int code )
{
	switch (code)
	{
		case 200: return "OK";
		case 400: return "Bad Request";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		default:  return "Unknown";
	}
}
