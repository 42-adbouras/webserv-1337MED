#include "../../includes/response.hpp"
#include "../../includes/request.hpp"

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

Location getLocation( ServerEntry *_srvEntry, Request& request, Response& response ) {
	str path = request.getPath();

	std::vector<Location> locations = _srvEntry->_locations;
	std::vector<str> lcts;
	for(size_t i=0; i < locations.size(); ++i) {
		if (startsWith(path, locations[i]._path))
			lcts.push_back(locations[i]._path);
	}
	if (lcts.empty()) {
		getSrvErrorPage(response, _srvEntry, NOT_FOUND);
		return Location();
	}
	std::vector<str>::iterator largest = std::max_element(lcts.begin(), lcts.end());
	request.setLocation(*largest);
	for(size_t i=0; i<locations.size(); ++i) {
		if(locations[i]._path == *largest)
			return locations[i];
	}
	return Location();
}
