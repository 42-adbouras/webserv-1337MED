#ifndef __MULTIPART_PARSER_HPP__
#define __MULTIPART_PARSER_HPP__

#include "TypeDefs.hpp"
#include "request.hpp"

class MultipartParser {
public:
	enum Result {
		NEED_MORE,
		COMPLETE,
		ERROR
	};

	MultipartParser( void );
	~MultipartParser();

	void init(const str& contentTypeHeader, const str& uploadDir, size_t max_body_size);

	Result feed( const char* data, size_t len );

	Result finish( void );

	bool isActive( void ) const;
	bool hasError( void ) const;
private:
	enum State {
		ST_EXPECT_BOUNDARY,
		ST_PART_HEADERS,
		ST_PART_BODY,
		ST_DONE
	};

	Result processBoundary( void );
	Result processPartHeaders( void );
	Result processPartBody( size_t available );

	void closeCurrentFile( void );
	bool openNewFile( const str& filename );

	bool	_active;
	bool	_error;
	State	_state;
	str	_buffer;
	str	_boundary;
	str	_closingBoundary;
	str	_uploadDir;
	size_t	_maxBodySize;
	size_t	_bytesReceived;

	int	_curFd;
	str	_curFilename;

	str   _headerBuffer;
};

#endif