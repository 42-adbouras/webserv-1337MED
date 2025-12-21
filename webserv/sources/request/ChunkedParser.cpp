#include "ChunkedParser.hpp"

ChunkedParser::ChunkedParser( void ) : _active(false), _error(false)
										,_state(ST_CHUNK_SIZE), _currentChunkSize(0)
										,_bytesWritten(0), _outputFd(-1) { }
ChunkedParser::~ChunkedParser() { closeOutput(); }

void ChunkedParser::init( int outputFd ) {
	_active = true;
	_error = false;
	_state = ST_CHUNK_SIZE;
	_buffer.clear();
	_currentChunkSize = 0;
	_bytesWritten = 0;
	_outputFd = outputFd;
}

ChunkedParser::Result ChunkedParser::feed( const char* data, size_t len ) {
	if (!_active || _error)
		return ERROR;

	_buffer.append(data, len);

	for (;;) {
		Result r = NEED_MORE;
		int stateBefore = _state;
		size_t bufferSizeBefore = _buffer.size();

		if (_state == ST_CHUNK_SIZE)
			r = processChunkSize();
		else if (_state == ST_CHUNK_DATA)
			r = processChunkData();
		else if (_state == ST_CHUNK_END)
			r = processChunkEnd();
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

ChunkedParser::Result ChunkedParser::processChunkSize() {
	str::size_type pos = _buffer.find("\r\n");
	if (pos == str::npos) {
		return NEED_MORE;
	}

	str hex = _buffer.substr(0, pos);
	_buffer.erase(0, pos + 2);

	sstream ss;
	ss << std::hex << hex;
	if (!(ss >> _currentChunkSize))
		return ERROR;

	_state = ST_CHUNK_DATA;
	_bytesWritten = 0;
	return NEED_MORE;
}

ChunkedParser::Result ChunkedParser::processChunkData() {
	size_t remainingInChunk = _currentChunkSize - _bytesWritten;
	if (_buffer.empty())
		return NEED_MORE;

	size_t toWrite = std::min(_buffer.size(), remainingInChunk);

	if (_outputFd != -1 && toWrite > 0) {
		ssize_t bytes = write(_outputFd, _buffer.data(), toWrite);
		
		if (bytes < 0)
			return ERROR;
		if (bytes != static_cast<ssize_t>(toWrite))
			return ERROR;
		_bytesWritten += bytes;
	} else {
		_bytesWritten += toWrite;
	}

	_buffer.erase(0, toWrite);

	if (_bytesWritten >= _currentChunkSize)
		_state = ST_CHUNK_END;

	return NEED_MORE;
}

ChunkedParser::Result ChunkedParser::processChunkEnd() {
	if (_buffer.size() < 2)
		return NEED_MORE;

	if (_buffer.substr(0, 2) != "\r\n")
		return ERROR;

	_buffer.erase(0, 2);

	if (_currentChunkSize == 0) {
		_state = ST_DONE;
		return COMPLETE;
	} else {
		_state = ST_CHUNK_SIZE;
		return NEED_MORE;
	}
}

ChunkedParser::Result ChunkedParser::finish() {
	if (_error)
		return ERROR;
	if (_state != ST_DONE) {
		_error = true;
		return ERROR;
	}
	closeOutput();
	_active = false;
	return COMPLETE;
}

void ChunkedParser::closeOutput() {
	if (_outputFd != -1) {
		close(_outputFd);
		_outputFd = -1;
	}
}

bool ChunkedParser::isActive() const { return _active; }
bool ChunkedParser::hasError() const { return _error; }

size_t ChunkedParser::getBytesWritten() const { return _bytesWritten; }