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

Location getLocation( ServerEntry *_srvEntry, Request& request ) {
	str path = request.getPath();

	std::vector<Location> locations = _srvEntry->_locations;
	std::vector<str> lcts;
	for(size_t i=0; i < locations.size(); ++i) {
		if (startsWith(path, locations[i]._path))
			lcts.push_back(locations[i]._path);
	}
	if (lcts.empty())
		throw Response::ResponseException("No matching location found");
	std::vector<str>::iterator largest = std::max_element(lcts.begin(), lcts.end());
	for(size_t i=0; i<locations.size(); ++i) {
		if(locations[i]._path == *largest)
			return locations[i];
	}
	return locations[0];
}
