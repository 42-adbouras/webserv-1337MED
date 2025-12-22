#include "../includes/multipartParser.hpp"

MultipartParser::MultipartParser( void ) : _active(false), _error(false), _state(ST_EXPECT_BOUNDARY)
											, _bytesReceived(0), _curFd(-1) { }

MultipartParser::~MultipartParser( ) { closeCurrentFile(); }

void MultipartParser::init( const str& contentTypeHeader, const str& uploadDir, size_t max_body_size ) {
	str::size_type pos = contentTypeHeader.find("boundary=");
	if (pos == str::npos) {
		_error = true;
		return;
	}
	str boundary = contentTypeHeader.substr(pos + 9);
	_boundary = "--" + boundary;
	_closingBoundary = _boundary + "--";
	_uploadDir = uploadDir;
	if (!_uploadDir.empty() && _uploadDir[_uploadDir.size() - 1] != '/')
		_uploadDir += '/';
	
	_maxBodySize = max_body_size;
	_bytesReceived = 0;
	_active = true;
	_state = ST_EXPECT_BOUNDARY;
	_buffer.clear();
	_headerBuffer.clear();
	_error = false;
}

MultipartParser::Result MultipartParser::feed( const char* data, size_t len ) {
	if (!_active || _error)
		return ERROR;
	_buffer.append(data, len);
	_bytesReceived += len;

	if (_bytesReceived > _maxBodySize) {
		_error = true;
		return ERROR;
	}

	for (;;) {
		Result r = NEED_MORE;
		int stateBefore = _state;
		size_t bufferSizeBefore = _buffer.size();
		if (_state == ST_EXPECT_BOUNDARY)
			r = processBoundary();
		else if (_state == ST_PART_HEADERS)
			r = processPartHeaders();
		else if (_state == ST_PART_BODY)
			r = processPartBody(_buffer.size());
		else if (_state == ST_DONE)
			return COMPLETE;

		if (r == ERROR) {
			_error = true;
			return ERROR;
		}
		if (r == COMPLETE)
			return COMPLETE;

		bool stateChanged = (_state != stateBefore);
		bool dataConsumed = (_buffer.size() < bufferSizeBefore);
		
		if (!stateChanged && !dataConsumed)
			break;

		if (_buffer.empty())
			break;
	}
	
	return NEED_MORE;
}

MultipartParser::Result MultipartParser::processBoundary( void ) {
	str::size_type pos = _buffer.find(_boundary);
	if (pos == str::npos) {
		pos = _buffer.find(_closingBoundary);
		if (pos == str::npos)
			return NEED_MORE;
	}

	_buffer.erase(0, pos + _boundary.size());
	if (_buffer.compare(0, 2, "--") == 0) {
		closeCurrentFile();
		_state = ST_DONE;
		return COMPLETE;
	}
	if (_buffer.size() < 2 || _buffer.substr(0, 2) != "\r\n") {
		_error = true;
		return ERROR;
	}
	_buffer.erase(0, 2);

	closeCurrentFile();
	_headerBuffer.clear();
	_state = ST_PART_HEADERS;
	return NEED_MORE;
}

MultipartParser::Result MultipartParser::processPartHeaders( void ) {
	str::size_type pos = _buffer.find("\r\n\r\n");
	if (pos == str::npos)
		return NEED_MORE;
	_headerBuffer = _buffer.substr(0, pos);
	_buffer.erase(0, pos + 4);

	_currentFile = UploadedFile();

	str::size_type fnPos = _headerBuffer.find("filename=\"");
	str originalFilename;
	if (fnPos != str::npos) {
		fnPos += 10;
		str::size_type end = _headerBuffer.find("\"", fnPos);
		if (end != str::npos) {
			originalFilename = _headerBuffer.substr(fnPos, end - fnPos);
			_currentFile.originalName = originalFilename;
		}
	}

	str contentType;
	str::size_type ctPos = _headerBuffer.find("Content-Type:");
	if (ctPos != str::npos) {
		ctPos += 13;
		str::size_type ctEnd = _headerBuffer.find("\r\n", ctPos);
		if (ctEnd != str::npos) {
			contentType = _headerBuffer.substr(ctPos, ctEnd - ctPos);
			str::size_type start = contentType.find_first_not_of(" \t");
			if (start != str::npos) {
				contentType = contentType.substr(start);
			}
			_currentFile.contentType = contentType;
		}
	}

	if (!_currentFile.originalName.empty()) {
		_curFilename = _currentFile.originalName;
		_currentFile.fullPath = _uploadDir + _currentFile.originalName;
	} else {
		_curFilename = FilenameGenerator::generate(originalFilename, _currentFile.contentType);
		_currentFile.generatedName = _curFilename;
		_currentFile.fullPath = _uploadDir + _curFilename;
	}

	if (!openNewFile(_curFilename))
		return ERROR;

	_state = ST_PART_BODY;
	return NEED_MORE;
}

MultipartParser::Result MultipartParser::processPartBody( size_t available ) {
	size_t boundPos = _buffer.find(_boundary);
	if (boundPos == std::string::npos)
		boundPos = _buffer.find(_closingBoundary);

	if (boundPos == std::string::npos) {
		size_t safe = (available > _boundary.size()) ? available - _boundary.size() : 0;
		if (safe > 0 && _curFd != -1) {
			ssize_t written = write(_curFd, _buffer.data(), safe);
			if (written < 0 || written != static_cast<ssize_t>(safe)) {
				_error = true;
				return ERROR;
			}
			_buffer.erase(0, safe);
		}
		return NEED_MORE;
	}

	if (boundPos >= 2 && _buffer.substr(boundPos - 2, 2) == "\r\n") {
		if (_curFd != -1) {
			ssize_t written = write(_curFd, _buffer.data(), boundPos - 2);
			if (written < 0 || written != static_cast<ssize_t>(boundPos - 2)) {
				_error = true;
				return ERROR;
			}
		}
		_buffer.erase(0, boundPos);
		_state = ST_EXPECT_BOUNDARY;
		return NEED_MORE;
	}

	_error = true;
	return ERROR;
}

void MultipartParser::closeCurrentFile( void ) {
	if (_curFd != -1) {
		close(_curFd);
		_curFd = -1;
	}
}

bool MultipartParser::openNewFile( const str& filename ) {
	if (filename.empty())
		return false;

	str fullPath = _uploadDir + filename;
	_curFd = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (_curFd == -1)
		return false;
	_curFilename = filename;
	return true;
}

const std::vector<MultipartParser::UploadedFile>& MultipartParser::getUploadedFiles( void ) const {
	return _uploadedFiles;
}

void MultipartParser::cleanupAllFiles( void ) {
	for (size_t i = 0; i < _uploadedFiles.size(); i++) {
		if (!_uploadedFiles[i].fullPath.empty())
			std::remove(_uploadedFiles[i].fullPath.c_str());
	}
	_uploadedFiles.clear();
}

bool MultipartParser::isActive( void ) const { return _active; }
bool MultipartParser::hasError( void ) const { return _error; }