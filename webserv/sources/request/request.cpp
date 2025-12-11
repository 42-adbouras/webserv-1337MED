#include "../../includes/request.hpp"
#include "../../includes/response.hpp"
#include "Client.hpp"

Request::Request( void )
	: _method()
	, _Uri()
	, _path()
	, _version()
	, _body()
	, _buffer()
	, _location()
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
		this->_location = req._location;
	}
	return *this;
}

const str& Request::getMethod( void ) const { return _method; }
const str& Request::getreqTarget( void ) const { return _Uri; }
const str& Request::getVersion( void ) const { return _version; }
const QueryMap& Request::getQueryParams( void ) const { return _queryParams; }
const HeadersMap& Request::getHeaders( void ) const { return _headers; }
const str& Request::getBody( void ) const { return _body; }
const str& Request::getPath( void ) const { return _path; }
const std::vector<char>& Request::getBuffer( void ) const { return _buffer; }
const str& Request::getUri( void ) const { return _Uri; }
const str& Request::getLocation( void ) const { return _location; }
void Request::setBuffer( std::vector<char> buffer ) {
	_buffer = buffer;
}
void Request::setLocation( str& location ) {
	_location = location;
}
void Request::setPath( const str& path ) {
	_path = path;
}
void Request::parseRequestLine( str& input ) {
	sstream stream(input);

	stream >> _method >> _Uri >> _version;
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
	
	_path = normalizePath(path.substr(0, pos));
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

void initPath( Request& request ) {
	str uri = request.getUri();
	str::size_type pos = uri.find('?');
	if (pos != str::npos)
		request.setPath(normalizePath(uri.substr(0, pos)));
	else
		request.setPath(normalizePath(uri));
	request.setPath(urlDecode(uri));
}

bool Request::requestLineErrors( Response& response, ServerEntry* _srvEntry ) {
	if (!_method.length() || !_Uri.length() || !_version.length()) {
		getSrvErrorPage(response, _srvEntry, BAD_REQUEST);
		return false;
	}
	if (!is_valid_method( _method )) {
		getSrvErrorPage(response, _srvEntry, NOT_IMPLEMENTED);
		return false;
	}
	if (_version != "HTTP/1.1") {
		getSrvErrorPage(response, _srvEntry, HTTP_VERSION_NOT_SUPPORTED);
		return false;
	}
	if (_Uri.length() > 2048) {
		getSrvErrorPage(response, _srvEntry, URI_TOO_LONG);
		return false;
	}
	if (!parse_query_params( _Uri ) || !UriAllowedChars( _Uri )) {
		getSrvErrorPage(response, _srvEntry, BAD_REQUEST);
		return false;
	}

	return true;
}

static str trim( const str& s ) {
	size_t start = s.find_first_not_of(" \t");
	size_t end = s.find_last_not_of(" \t");
	return (start == str::npos) ? "" : s.substr(start, end - start + 1);
}

void Request::initHeaders( str& input ) {
	sstream stream(input);
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

void Request::setBody( str& body ) {
	_body = body;
}

ServerEntry* getSrvBlock( serverBlockHint& _srvBlockHint, Request& request) {
	serverBlockHint::iterator it = _srvBlockHint.begin();
	while(it != _srvBlockHint.end()) {
		if (it->first->_serverName == getHost(request.getHeaders()))
			return it->second;
		++it;
	}
	return _srvBlockHint.begin()->second;
}

void checkMethod( ServerEntry *_srvEntry, Request& request, Response& response, str& src, Client& client ) {
	if (request.getMethod() == "DELETE")
		deleteHandler(_srvEntry, request, response, src, client);
	else if (request.getMethod() == "GET")
		getHandler(_srvEntry, request, response, src, client);
	else if (request.getMethod() == "POST")
		postHandler(_srvEntry, request, response, src, client);
}

void processClientRequest( Client& client ) {
	Response response;

	Request request = client.getRequest();
	std::vector<char> bufferVec = request.getBuffer();
	str buffer(bufferVec.begin(), bufferVec.end());
	ServerEntry* _srvEntry = client.getResponse().getSrvEntry();
	bool reqFlg = request.requestLineErrors( response, _srvEntry );
	initPath(request);
	if (!reqFlg) {
		client.setClientState(CS_KEEPALIVE);
	} else {
		if (requestErrors(request, response, _srvEntry)) {
			str source = getSource(request, _srvEntry, response);
			response.setSrc(source);
			checkMethod( _srvEntry, client.getRequest(), response, source, client );
		}
	}
	client.setResponse(response);
}

void requestHandler( Client& client ) {
	if (client._reqInfo.reqStatus == CS_NEW)
		client._reqInfo.reqStatus = CS_READING;
	std::vector<char> newChunk = client.getRequest().getBuffer();
	if (newChunk.empty())
		return;

	client.getLeftover().append(newChunk.begin(), newChunk.end());

	while (true)
	{
		if (client._state == PARSING_HEADERS) {
			size_t headersEndPos = client.getLeftover().find("\r\n\r\n");
			if (headersEndPos == str::npos) {
				if (client.getLeftover().size() > 8192) {
					client.getResponse().setStatus(RANGE_NOT_SATISFIABLE);
					client._state = REQUEST_COMPLETE;
				}
				return;
			}
			str rawHeaders = client.getLeftover().substr(0, headersEndPos + 4);

			client.getRequest().parseRequestLine(rawHeaders);
			initPath(client.getRequest());
			client.getRequest().initHeaders(rawHeaders);
			HeadersMap headers = client.getRequest().getHeaders();
			ServerEntry* _srvEntry = getSrvBlock( client._serverBlockHint, client.getRequest() );
			client.getResponse().setSrvEntry(_srvEntry);
			str source = getSource(client.getRequest(), _srvEntry, client.getResponse());
			client.getResponse().setSrc(source);

			// set isChunked and content-length
			str cl = headers["Content-Length"];
			str te = headers["Transfer-Encoding"];

			client.setIsChunked(te.find("chunked") != str::npos);
			client.setExpectedBodyLength(cl.empty() ? 0 : sToSize_t(cl));

			if (client.getExpectedBodyLength() > _srvEntry->_maxBodySize) {
				client.getResponse().setStatus(CONTENT_TOO_LARGE);
				client._state = REQUEST_COMPLETE;
				return;
			}
			client.getLeftover().erase(0, headersEndPos + 4);

			client._state = PARSING_BODY;
		}

		if (client._state == PARSING_BODY) {
			if (client.getRequest().getMethod() == "POST") {
				char templ[] = "/tmp/webserv_upload_XXXXX";
				int fd = mkstemp(templ);
				if (fd == -1) {
					client.getResponse().setStatus(INTERNAL_SERVER_ERROR);
					client._state = REQUEST_COMPLETE;
					return;
				}
				client._uploadFd = fd;
				client._uploadPath = templ;
				client._isStreamingUpload = true;
				HeadersMap headers = client.getRequest().getHeaders();
				if (!client.getIsChunked()) {
					str ct = headers["Content-Type"];
					if (ct.find("multipart/form-data") == str::npos) {
						// single file TO-DO
						if (client.getLeftover().size() >= client.getExpectedBodyLength()) {
							str body = client.getLeftover().substr(0, client.getExpectedBodyLength());
							client.getRequest().setBody(body);

							client.getLeftover().erase(0, client.getExpectedBodyLength());

							client._state = REQUEST_COMPLETE;
							break;
						} else {
							// multipart
							str multipartHeader = headers["Content-Type"];
							str::size_type boundaryPos = multipartHeader.find("boundary=");
							if (boundaryPos == str::npos) {
								client.getResponse().setStatus(BAD_REQUEST);
								client._state = REQUEST_COMPLETE;
								return;
							}
							str boundary = multipartHeader.substr(boundaryPos);
						}
					std::cout << "CONTENT_TYPE: " << ct << std::endl;
					} else {
						// waiting for more bytes
						return;
					}
				} else {
					size_t pos = 0;
					str body;
					while (true) {
						str::size_type chunkSize = client.getLeftover().find("\r\n", pos);
						if (chunkSize == str::npos)
							return;
						str hex = client.getLeftover().substr(pos, chunkSize - pos);
						sstream ss;
						ss << std::hex << hex;
						size_t x = 0;
						ss >> x;
						if (client.getLeftover().size() < chunkSize + 2 + x + 2)
							return;
						if (x == 0) {
							pos = chunkSize + 2;
							client._state = REQUEST_COMPLETE;
							break;
						}
						// write(fd, client.getLeftover().substr(chunkSize + 2).c_str(), client.getLeftover().substr(chunkSize + 2).size());
						body.append(client.getLeftover().substr(chunkSize + 2, x));
						pos = chunkSize + 2 + x + 2;
					}
					client.getRequest().setBody(body);
					client.getLeftover().erase(0, pos);
					break;
				}
			}
			client._state = REQUEST_COMPLETE;
			break;
		}
	}

	if (client._state == REQUEST_COMPLETE) {
		client._reqInfo.reqStatus = CS_READING_DONE;
		processClientRequest( client );

		client.getLeftover().clear();
		client._state = PARSING_HEADERS;
		client.setExpectedBodyLength(0);
		client.setIsChunked(false);
	}
}
