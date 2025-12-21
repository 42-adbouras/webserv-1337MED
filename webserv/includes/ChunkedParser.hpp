#ifndef __CHUNKED_PARSER_HPP__
#define __CHUNKED_PARSER_HPP__

#include "TypeDefs.hpp"
#include "request.hpp"

class ChunkedParser {
public:
	enum Result {
		NEED_MORE,
		COMPLETE,
		ERROR
	};

	ChunkedParser( void );
	~ChunkedParser();

	void init( int outputFd );

	Result feed( const char* data, size_t len );

	Result finish();

	bool isActive() const;
	bool hasError() const;

	size_t getBytesWritten() const;
private:
	enum State {
		ST_CHUNK_SIZE,
		ST_CHUNK_DATA,
		ST_CHUNK_END,
		ST_DONE
	};

	Result processChunkSize();
	Result processChunkData();
	Result processChunkEnd();

	void closeOutput();

	bool	_active;
	bool	_error;
	State	_state;
	str	_buffer;
	size_t	_currentChunkSize;
	size_t	_bytesWritten;
	int	_outputFd;
};

#endif