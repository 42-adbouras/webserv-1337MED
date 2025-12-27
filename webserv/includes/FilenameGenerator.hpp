#ifndef __FILENAME_GENERATOR_HPP__
#define __FILENAME_GENERATOR_HPP__

#include "./TypeDefs.hpp"

class FilenameGenerator {
private:
	static int _counter;

	static str extractExt( const str& filename );
	static str getExtensionFromContentType( const str& contentType );
public:
	static str generate( const str& originalFilename, const str& contentType );
	static str generateFromContentType( const str& contentType );
};

#endif
