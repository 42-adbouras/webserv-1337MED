#include "../../includes/response.hpp"
#include "../../includes/serverHeader/ServerUtils.hpp"
#include "Client.hpp"

Response::Response( void )
	: _statusCode(0)
	, _statusText()
	, _version("HTTP/1.1")
	, _body()
	, _contentLength(0)
	, _source()
	, _streamFile(false)
	, _filePath()
	, _fileOffset(0)
	, _fileSize(0)
	, _bytesSent(0) {

		_headers["Server"] = "WebServer/0.0 (ait-server)";
		_headers["Connection"] = "keep-alive";
		flag = false;
		_srvEntry = NULL;
}

Response::~Response() { }
Response& Response::operator=( const Response& res ) {
	if (this != &res) {
		this->_statusCode = res._statusCode;
		this->_statusText = res._statusText;
		this->_version = res._version;
		this->_body = res._body;
		this->_contentLength = res._contentLength;
		this->_headers = res._headers;
		this->_source = res._source;
		this->_srvEntry = res._srvEntry;
		this->flag = res.flag;
		this->_streamFile = res._streamFile;
		this->_filePath = res._filePath;
		this->_fileOffset = res._fileOffset;
		this->_fileSize = res._fileSize;
		this->_bytesSent = res._bytesSent;
	}
	return *this;
}

const int& Response::getStatusCode( void ) const { return _statusCode; }
const str& Response::getStatusText( void ) const { return _statusText; }
const str& Response::getVersion( void ) const { return _version; }
const str& Response::getBody( void ) const { return _body; }
const size_t& Response::getContentLength( void ) const { return _contentLength; }
HeadersMap& Response::getHeaders( void ) { return _headers; }
str& Response::getSrc( void ) { return _source; }
ServerEntry* Response::getSrvEntry( void ) const { return _srvEntry; }
bool Response::getFlag( void ) const { return flag; }

void Response::setSrc( const str& source ) {
	_source = source;
}
void Response::setSrvEntry( ServerEntry* srvEnt ) {
	_srvEntry = srvEnt;
}
void Response::setFlag( bool flg ) {
	flag = flg;
}

static const StatusEntry StatusEntries[] = {
	{200, "OK"},
	{201, "Created"},
	{204, "No Content"},
	{206, "Partial Content"},
	{301, "Moved Permanently"},
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{409, "Conflict"},
	{413, "Content Too Large"},
	{414, "URI Too Long"},
	{416, "Range Not Satisfiable"},
	{431, "Request Header Fields Too Large"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{505, "HTTP Version Not Supported"}
};

static const std::map<int, str>& getStatusMap() {
	static std::map<int, str> StatusMap;
	if (StatusMap.empty()) {
		for (size_t i=0; i<sizeof(StatusEntries)/sizeof(StatusEntry); ++i)
			StatusMap[StatusEntries[i].code] = StatusEntries[i].message;
	}
	return StatusMap;
}

std::map<int, str> statusMap = getStatusMap();

void Response::setStatus( int code ) {
	std::map<int, str>::const_iterator it = statusMap.find(code);
	if ( it != statusMap.end() ) {
		_statusCode = code;
		_statusText = it->second;
	}
}

void Response::addHeaders( const str& key, const str& value ) {
	_headers[key] = value;
}

void Response::setBody( const str& bodyData ) {
	_body = bodyData;
	_contentLength = _body.length();
}

str Response::generate( void ) {
	std::ostringstream ss;
	ss << "HTTP/1.1 " << _statusCode << " " << _statusText << "\r\n";

	ss << "Date: " << getDateHeader() << "\r\n";

	for (HeadersMap::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		ss << it->first << ": " << it->second << "\r\n";

	if (!_streamFile && !_body.empty()) {
		ss << "Content-Length: " << _body.size() << "\r\n\r\n";
		ss << _body;
	} else {
		ss << "\r\n";
	}

	return ss.str();
}

Range parseRangeHeader(const std::string& rangeHeader, long long fileSize) {
	Range r = {-1, -1, false, ""};

	if (rangeHeader.size() < 7 || rangeHeader.substr(0, 6) != "bytes=") {
		r.error = "Not bytes=";
		return r;
	}

	std::string rangeSpec = rangeHeader.substr(6);

	size_t dashPos = rangeSpec.find('-');
	if (dashPos == std::string::npos) {
		r.error = "No dash found";
		return r;
	}

	std::string first  = rangeSpec.substr(0, dashPos);
	std::string second = rangeSpec.substr(dashPos + 1);

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
		if (r.end <= 0 || r.end > fileSize) {
			r.error = "Invalid suffix length";
			return r;
		}
		r.start = fileSize - r.end;
		r.end   = fileSize - 1;
	}

	if (r.start != -1 && r.start >= fileSize) 	{
		r.valid = false;
		r.error = "Start beyond EOF";
		return r;
	}

	if (r.end == -1)
		r.end = fileSize - 1;

	if (r.start == -1)
		r.start = 0;

	r.start = std::max(0LL, std::min(r.start, fileSize - 1));
	r.end   = std::min(r.end, fileSize - 1);

	r.valid = true;
	return r;
}

void sendResponse(Client& client) {
	Response& response = client.getResponse();

	ServerEntry* _srvEntry = getSrvBlock(client._serverBlockHint, client.getRequest());
	const HeadersMap& reqHeaders = client.getRequest().getHeaders();
	if (client._sendInfo.resStatus == CS_START_SEND) {
		client.setStartTime(std::time(NULL));
		client.setTimeOut(CLIENT_BODY_TIMEOUT);
		str headers = response.generate();
		client._sendInfo.buff.assign(headers.begin(), headers.end());
		client._sendInfo.resStatus = CS_WRITING;
		if (response.getFlag() && !reqHeaders.count("Range")) {
			client._sendInfo.resStatus = CS_WRITING_DONE;
			client._sendInfo.connectionState = CLOSED;
			return;
		}
	}

	if (response._streamFile) {
		if (response._bytesSent == 0) {
			if (reqHeaders.count("Range")) {
				Range r = parseRangeHeader(reqHeaders.at("Range"), response._fileSize);
				if (r.valid) {
					response.setStatus(PARTIAL_CONTENT);
					response.addHeaders("Content-Range",
						"bytes " + toString(r.start) + "-" + toString(r.end) + "/" + toString(response._fileSize));
					response.addHeaders("Content-Length", toString(r.end - r.start + 1));

					str newHeaders = response.generate();
					client._sendInfo.buff.assign(newHeaders.begin(), newHeaders.end());
					response._fileOffset = r.start;
				} else {
					getSrvErrorPage(response, _srvEntry, RANGE_NOT_SATISFIABLE);
					response._streamFile = false;
					response.getHeaders().erase("Content-Length");
					str errorResponse = response.generate();
					client._sendInfo.buff.assign(errorResponse.begin(), errorResponse.end());
					client._sendInfo.resStatus = CS_WRITING_DONE;
					return;
				}
			}
		}

		if (client._sendInfo.fd <= 0) {
			client._sendInfo.fd = open(response._filePath.c_str(), O_RDONLY);
			if (client._sendInfo.fd == -1) {
				getSrvErrorPage(response, _srvEntry, INTERNAL_SERVER_ERROR);
				response._streamFile = false;
				response.getHeaders().erase("Content-Length");
				str errorResponse = response.generate();
				client._sendInfo.buff.assign(errorResponse.begin(), errorResponse.end());
				client._sendInfo.resStatus = CS_WRITING_DONE;
				return;
			}
		}

		const size_t CHUNK_SIZE = SRV_SEND_BUFFER;
		char buffer[CHUNK_SIZE];
		off_t offset = response._fileOffset;
		ssize_t toRead = CHUNK_SIZE;

		if (response._fileSize - offset < (off_t)CHUNK_SIZE)
			toRead = response._fileSize - offset;

		ssize_t bytesRead = pread(client._sendInfo.fd, buffer, toRead, offset);
		if (bytesRead <= 0) {
			if (bytesRead == 0) {
				close(client._sendInfo.fd);
				client._sendInfo.fd = -1;
				client._sendInfo.resStatus = CS_WRITING_DONE;
			} else {
				close(client._sendInfo.fd);
				client._sendInfo.fd = -1;
				getSrvErrorPage(response, _srvEntry, INTERNAL_SERVER_ERROR);
				response._streamFile = false;
				response.getHeaders().erase("Content-Length");
				str errorResponse = response.generate();
				client._sendInfo.buff.clear();
				client._sendInfo.buff.assign(errorResponse.begin(), errorResponse.end());
				client._sendInfo.resStatus = CS_WRITING_DONE;
			}
			return;
		}
		client._sendInfo.buff.insert(client._sendInfo.buff.end(), buffer, buffer + bytesRead);
		response._fileOffset += bytesRead;
		response._bytesSent += bytesRead;

		return;
	}

	if (!response._streamFile && !response.getBody().empty() && client._sendInfo.buff.empty()) {
		str fullResponse = response.generate();
		client._sendInfo.buff.assign(fullResponse.begin(), fullResponse.end());
		client._sendInfo.resStatus = CS_WRITING_DONE;
	}

	// probably 204/304/201
	if (client._sendInfo.buff.empty()) {
		client._sendInfo.resStatus = CS_WRITING_DONE;
	}
}
