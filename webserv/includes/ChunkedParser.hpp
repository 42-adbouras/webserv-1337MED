#ifndef __CHUNKED_PARSER_HPP__
#define __CHUNKED_PARSER_HPP__

#include "TypeDefs.hpp"
#include "request.hpp"

class ChunkedParser {
public:
	enum Result {
		NEED_MORE,
		COMPLETE,
		ERROR,
		MAXERROR
	};

	ChunkedParser( void );
	~ChunkedParser();

	void init( int outputFd, size_t maxBodySize );

	Result feed( const char* data, size_t len );

	bool isActive( void ) const;
	bool hasError( void ) const;

	size_t getBytesWritten( void ) const;
private:
	enum State {
		ST_CHUNK_SIZE,
		ST_CHUNK_DATA,
		ST_CHUNK_END,
		ST_DONE
	};

	Result processChunkSize( void );
	Result processChunkData( void );
	Result processChunkEnd( void );

	void closeOutput( void );

	bool	_active;
	bool	_error;
	State	_state;
	str	_buffer;
	size_t	_currentChunkSize;
	size_t	_bytesWritten;
	int	_outputFd;
	size_t _totalBytesWritten;
	size_t _maxBodySize;
};

#endif