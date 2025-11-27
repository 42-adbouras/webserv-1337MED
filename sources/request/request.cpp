#include "../../includes/request.hpp"
#include "../../includes/response.hpp"
#include "../../includes/serverHeader/Client.hpp"

Request::Request( void )
	: _method()
	, _Uri()
	, _path()
	, _version()
	, _body()
	, _buffer()
	, _location()
	// , _leftover()
	// , _expectedBodyLength(0)
	// , _isChunked(false)
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
		// this->_leftover = req._leftover;
		// this->_expectedBodyLength = req._expectedBodyLength;
		// this->_isChunked = req._isChunked;
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
// size_t Request::getExpectedBodyLength( void ) const { return _expectedBodyLength; }
// bool Request::getIsChunked( void ) const { return _isChunked; }
// const str& Request::getLeftover( void ) const { return _leftover; }
void Request::setBuffer( std::vector<char> buffer ) {
	_buffer = buffer;
}
void Request::setLocation( str& location ) {
	_location = location;
}
void Request::setPath( const str& path ) {
	_path = path;
}
// void Request::setExpectedBodyLength( size_t lgth ) {
// 	_expectedBodyLength = lgth;
// }
// void Request::setIsChunked( bool chunked ) {
// 	_isChunked = chunked;
// }
// void Request::setLeftover( str& leftover ) {
// 	_leftover = leftover;
// }

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

bool Request::parseReqline( str& input, Response& response, ServerEntry* _srvEntry ) {
	str raw = str(input);

	sstream stream(raw);
	if(!(stream >> _method >> _Uri >> _version)) {
		getSrvErrorPage(response, _srvEntry, BAD_REQUEST);
		return false;
	}
	if(!is_valid_method( _method )) {
		getSrvErrorPage(response, _srvEntry, NOT_IMPLEMENTED);
		return false;
	}
	if(_version != "HTTP/1.1") {
		getSrvErrorPage(response, _srvEntry, HTTP_VERSION_NOT_SUPPORTED);
		return false;
	}
	if (_Uri.length() > 2048) {
		getSrvErrorPage(response, _srvEntry, URI_TOO_LONG);
		return false;
	}
	if(!parse_query_params( _Uri ) || !UriAllowedChars( _Uri )) {
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

void Request::initBody( str& input ) {
	str raw(input);
	str::size_type pos = raw.find("\r\n\r\n");

	if (pos != str::npos)
		_body = raw.substr(pos + 4);
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
	ServerEntry* _srvEntry = getSrvBlock( client._serverBlockHint, request );
	bool reqFlg = request.parseReqline( buffer, response, _srvEntry );
	initPath(request);
	if (!reqFlg) {
		client.setClientState(CS_KEEPALIVE);
	} else {
		if (requestErrors(request, response, _srvEntry)) {
			str source = getSource(request, _srvEntry, response);
			response.setSrc(source);
			// std::cout << "--Source-- : " << source << std::endl;
			checkMethod( _srvEntry, request, response, source, client );
		}
	}
	client.setResponse(response);
}

void requestHandler( Client& client ) {
	// Request request;
	if (client._reqInfo.reqStatus == CS_NEW)
		client._reqInfo.reqStatus = CS_READING;
	std::vector<char> newChunk = client.getRequest().getBuffer();
	if (newChunk.empty())
		return;

	str leftover = client.getLeftover();
	leftover.append(newChunk.begin(), newChunk.end());
	client.setLeftover(leftover);

	while (true)
	{
		if (client._state == PARSING_HEADERS) {
			size_t headersEndPos = leftover.find("\r\n\r\n");
			if (headersEndPos == str::npos) {
				if (leftover.size() > 64*1024) {
					// 431 Fields ar too big; TO-DO
					client.setClientState(CS_DISCONNECT);
				}
				return;
			}
			str rawHeaders = leftover.substr(0, headersEndPos + 4);

			// request.initHeaders(rawHeaders);
			client.getRequest().initHeaders(rawHeaders);
			HeadersMap headers = client.getRequest().getHeaders();
			// HeadersMap headers = request.getHeaders();

			// set isChunked and content-length
			str cl = headers["Content-Length"];
			str te = headers["Transfer-Encoding"];

			client.setIsChunked(te.find("chunked") != str::npos);
			client.setExpectedBodyLength(cl.empty() ? 0 : atoi(cl.c_str()));

			leftover.erase(0, headersEndPos + 4);
			client.setLeftover(leftover);

			client._state = PARSING_BODY;
		}

		if (client._state == PARSING_BODY) {
			if (!client.getIsChunked()) {
				if (leftover.size() >= client.getExpectedBodyLength()) {
					str body = leftover.substr(0, client.getExpectedBodyLength());
					// request.initBody(body);
					client.getRequest().initBody(body);

					leftover.erase(0, client.getExpectedBodyLength());
					client.setLeftover(leftover);

					client._state = REQUEST_COMPLETE;
					break;
				} else {
					// waiting for more bytes
					return;
				}
			} else {
				// chunked TO-DO
			}
		}
	}

	if (client._state == REQUEST_COMPLETE) {
		client._reqInfo.reqStatus = CS_READING_DONE;
		// client.setRequest(request);
		processClientRequest( client );

		leftover.clear();
		client._state = PARSING_HEADERS;
		client.setExpectedBodyLength(0);
		client.setIsChunked(false);
	}

	// char buffer[30000];
	// ssize_t rByte;
	// client.setClientState(CS_READING);
	// std::cout << BG_BLUE << "recve fd " << client.getFd() << std::endl;

	// if((rByte = recv(client.getFd(), buffer, sizeof(buffer), 0)) > 0) {
	// 	buffer[rByte] = '\0';
	// 	client.setClientState(CS_KEEPALIVE);
	// }
	// else if(rByte == 0) {
	// 	client.setClientState(CS_DISCONNECT);
	// 	return ;
	// }
	// else {
	// 	if (errno == EWOULDBLOCK || errno == EAGAIN) /* no data now in non-blocking socket; return to poll() */
	// 	{
	// 		client.setClientState(CS_READING);
	// 		return ;
	// 	}
	// 	std::cerr << "recv failed fd=" << client.getFd() << ": " << strerror(errno) << std::endl;
	// 	client.setClientState(CS_DISCONNECT);
	// 	return ;
	// }
	

	// request.setBuffer( bufferVec );
	// request.initHeaders( buffer );
	// request.initBody( buffer );
}

struct Range {
	long long start;
	long long end;
	bool valid;
	str error;
};

Range parse_range_header(const std::string& range_header, long long file_size) {
	Range r = {-1, -1, false, ""};

	if (range_header.size() < 7 || range_header.substr(0, 6) != "bytes=") {
		r.error = "Not bytes=";
		return r;
	}

	std::string range_spec = range_header.substr(6);

	size_t dash_pos = range_spec.find('-');
	if (dash_pos == std::string::npos) {
		r.error = "No dash found";
		return r;
	}

	std::string first  = range_spec.substr(0, dash_pos);
	std::string second = range_spec.substr(dash_pos + 1);

	char* endptr;

	if (first.empty())
		r.start = -1;
	else {
		long long val = strtoll(first.c_str(), &endptr, 10);
		if (*endptr != '\0' || val < 0) {
			r.error = "Invalid start";
			return r;
		}
		r.start = val;
	}

	if (second.empty())
		r.end = -1;
	else {
		long long val = strtoll(second.c_str(), &endptr, 10);
		if (*endptr != '\0') {
			r.error = "Invalid end";
			return r;
		}
		r.end = val;
	}

	if (r.start != -1 && r.end != -1 && r.start > r.end) {
		r.error = "start > end";
		return r;
	}

	if (r.start == -1 && r.end != -1) {
		if (r.end <= 0 || r.end > file_size) {
			r.error = "Invalid suffix length";
			return r;
		}
		r.start = file_size - r.end;
		r.end   = file_size - 1;
	}

	if (r.start != -1 && r.start >= file_size) 	{
		r.valid = false;
		r.error = "Start beyond EOF";
		return r;
	}

	if (r.end == -1)
		r.end = file_size - 1;

	if (r.start == -1)
		r.start = 0;

	r.start = std::max(0LL, std::min(r.start, file_size - 1));
	r.end   = std::min(r.end, file_size - 1);

	r.valid = true;
	return r;
}

long long getFileSize( const str& src ) {
	struct stat st;

	if (stat(src.c_str(), &st) == -1)
		return -1;
	if (!S_ISREG(st.st_mode))
		return -1;

	return (long long)st.st_size;
}

template<typename T>
str toString(T n) {
	std::ostringstream ss;
	ss << n;
	return ss.str();
}

bool send_file_chunk(int client_socket, const std::string& filepath, long long start, long long end) {
	int fd = open(filepath.c_str(), O_RDONLY);
	if (fd == -1) {
		return false;
	}

	if (lseek(fd, start, SEEK_SET) == (off_t)-1) {
		close(fd);
		return false;
	}

	long long bytes_to_send = end - start + 1;
	long long bytes_sent_total = 0;

	const int CHUNK_SIZE = 8192;  // 8 KB → perfect balance
	char buffer[CHUNK_SIZE];

	while (bytes_sent_total < bytes_to_send) {
		long long remaining = bytes_to_send - bytes_sent_total;
		int to_read = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : (int)remaining;

		ssize_t bytes_read = read(fd, buffer, to_read);
		if (bytes_read <= 0) {
			if (bytes_read < 0) {
				// real error
				close(fd);
				return false;
			}
			break;
		}
		ssize_t bytes_sent = send(client_socket, buffer, bytes_read, 0);
		if (bytes_sent != bytes_read) {
			std::cout << RED << "!!!!!!!!!!!!!!!!!!!!!!" << RED << std::endl;
			close(fd);
			return false;
		}

		bytes_sent_total += bytes_sent;
	}

	close(fd);
	return true;
}

void send_response_headers(int client_socket, const Response& resp) {
	std::string headers = resp.generate();
	send(client_socket, headers.c_str(), headers.length(), 0);
}

void sendResponse( Client& client ) {
	Response response = client.getResponse();
	Request request = client.getRequest();
	HeadersMap hdrs = request.getHeaders();

	if (response.getFlag()) {
		long long fileSize = getFileSize(response.getSrc());

		bool is_range = false;
		Range r = {0, fileSize-1, true, ""};

		if (hdrs.find("Range") != hdrs.end()) {
			r = parse_range_header(hdrs.find("Range")->second, fileSize);
			if (!r.valid || r.start >= fileSize) {
				// response.setStatus(RANGE_NOT_SATISFIABLE);
				response.addHeaders("Content-Range", "bytes */" + toString(fileSize));
				getSrvErrorPage(response, response.getSrvEntry(), RANGE_NOT_SATISFIABLE);
				return;
			}
		is_range = true;
	}

	response.setStatus(is_range ? PARTIAL_CONTENT : OK);
	response.addHeaders("Content-Length", toString(r.end - r.start + 1));
	response.addHeaders("Content-Type", getContentType(response.getSrc()));

	if (is_range) {
		response.addHeaders("Content-Range", "bytes " + toString(r.start) + "-" + toString(r.end) + "/" + toString(fileSize));
	}
	send_response_headers(client.getFd(), response);
	send_file_chunk(client.getFd(), response.getSrc(), r.start, r.end);
	} else {
		str content = response.generate();
		send(client.getFd(), content.c_str(), content.length(), 0);
		client.setClientState(CS_KEEPALIVE);
	}
}