#include "../../includes/response.hpp"

std::deque<str> splitPath( const str& path ) {
	std::deque<str> segments;
	str::size_type pos = 0;

	str::size_type next = path.find('/', pos);
	while(next != str::npos) {
		if (next > pos)
			segments.push_back(path.substr(pos, next - pos));
		pos = next + 1;
		next = path.find('/', pos);
	}
	if (pos < path.length())
		segments.push_back(path.substr(pos));

	return segments;
}

void deleteHandler(ServerEntry *_srvEntry, Request& request, Response& response) {
	(void)_srvEntry;
	(void)response;
	str source = request.getPath();
	// std::cout << source << std::endl;
	std::deque<str> segments = splitPath(source);
	// std::deque<str>::iterator it = segments.begin();
	// while(it != segments.end()) {
	// 	std::cout << *it << std::endl;
	// 	++it;
	// }
}
