#include "../../includes/request.hpp"

bool UriAllowedChars( str& uri ) {
	str allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";

	for (str::size_type i=0; i < uri.length(); ++i) {
		if(allowedChars.find(uri[i]) == str::npos)
			return false;
	}
	return true;
}