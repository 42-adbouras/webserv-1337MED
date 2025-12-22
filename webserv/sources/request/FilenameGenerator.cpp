#include "../../includes/FilenameGenerator.hpp"

int FilenameGenerator::_counter = 0;

str FilenameGenerator::extractExt( const str& filename ) {
	str::size_type dotPos = filename.find('.');
	if (dotPos != str::npos && dotPos < filename.length() - 1) {
		str ext = filename.substr(dotPos);

		if (ext.length() > 10)
			ext = ext.substr(0, 10);
	}
	return "";
}

str FilenameGenerator::getExtensionFromContentType( const str& contentType ) {
	if (contentType.find("image/jpeg") != str::npos || 
		contentType.find("image/jpg") != str::npos)
		return ".jpg";
	if (contentType.find("image/png") != str::npos)
		return ".png";
	if (contentType.find("image/gif") != str::npos)
		return ".gif";
	if (contentType.find("image/webp") != str::npos)
		return ".webp";
	if (contentType.find("image/svg") != str::npos)
		return ".svg";

	if (contentType.find("application/pdf") != str::npos)
		return ".pdf";
	if (contentType.find("text/plain") != str::npos)
		return ".txt";
	if (contentType.find("text/html") != str::npos)
		return ".html";
	if (contentType.find("text/css") != str::npos)
		return ".css";
	if (contentType.find("application/msword") != str::npos)
		return ".doc";
	if (contentType.find("application/vnd.openxmlformats-officedocument.wordprocessingml") != str::npos)
		return ".docx";
	if (contentType.find("application/vnd.ms-excel") != str::npos)
		return ".xls";
	if (contentType.find("application/vnd.openxmlformats-officedocument.spreadsheetml") != str::npos)
		return ".xlsx";
	if (contentType.find("text/csv") != str::npos)
		return ".csv";

	if (contentType.find("application/javascript") != str::npos || 
		contentType.find("text/javascript") != str::npos)
		return ".js";
	if (contentType.find("application/json") != str::npos)
		return ".json";
	if (contentType.find("application/xml") != str::npos || 
		contentType.find("text/xml") != str::npos)
		return ".xml";

	if (contentType.find("application/zip") != str::npos)
		return ".zip";
	if (contentType.find("application/x-tar") != str::npos)
		return ".tar";
	if (contentType.find("application/gzip") != str::npos)
		return ".gz";
	if (contentType.find("application/x-rar") != str::npos)
		return ".rar";
	if (contentType.find("application/x-7z") != str::npos)
		return ".7z";

	if (contentType.find("video/mp4") != str::npos)
		return ".mp4";
	if (contentType.find("video/mpeg") != str::npos)
		return ".mpeg";
	if (contentType.find("video/webm") != str::npos)
		return ".webm";
	if (contentType.find("video/x-msvideo") != str::npos)
		return ".avi";
	if (contentType.find("audio/mpeg") != str::npos)
		return ".mp3";
	if (contentType.find("audio/wav") != str::npos)
		return ".wav";
	if (contentType.find("audio/ogg") != str::npos)
		return ".ogg";

	return ".bin";
}

str FilenameGenerator::generate( const str& originalFilename, const str& contentType ) {
	str extension;
	if (!originalFilename.empty())
		extension = extractExt(originalFilename);

	if (extension.empty() && !contentType.empty())
		extension = getExtensionFromContentType(contentType);

	if (extension.empty())
		extension = ".bin";

	struct timeval tv;
	gettimeofday(&tv, NULL);

	sstream oss;
	oss << "upload_" << tv.tv_sec << "_" << tv.tv_usec 
		<< "_" << (++_counter) << extension;

	return oss.str();
}

str FilenameGenerator::generateFromContentType( const str& contentType ) {
	return generate("", contentType);
}
