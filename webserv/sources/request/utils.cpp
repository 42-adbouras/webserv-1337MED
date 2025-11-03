#include "../../includes/request.hpp"

bool UriAllowedChars( str& uri ) {
	str allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";

	for (str::size_type i=0; i < uri.length(); ++i) {
		if(allowedChars.find(uri[i]) == str::npos)
			return false;
	}
	return true;
}

str getSource( const str& path ) {
	str result = path.substr(1);
	if (!result.empty() && result[result.length() - 1] == '/')
		result = result.substr(0, result.length() - 1);
	return result;
}

str getHost( const std::map<str, str>& headers ) {
	std::map<str, str>::const_iterator it = headers.find("Host");
	if (it != headers.end())
		return it->second;
	return "";
}

bool isNumber(str& s) {
	for (size_t i=0; i<s.length(); ++i) {
		if (!isdigit(s[i]))
			return false;
	}
	return true;
}