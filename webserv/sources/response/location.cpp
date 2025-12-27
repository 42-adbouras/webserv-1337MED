#include "../../includes/response.hpp"
#include "../../includes/request.hpp"
#include <algorithm>
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

Location getLocation( Request& request, Response& response ) {
	str path = request.getPath();

	std::vector<Location> locations = request.getSrvEntry()->_locations;
	std::vector<str> lcts;
	for(size_t i=0; i < locations.size(); ++i) {
		str loc = locations[i]._path;
		if (loc.size() > 1 && loc[loc.size() - 1] == '/')
			loc = loc.substr(0, loc.size() - 1);
		if (loc == "/") {
			lcts.push_back(loc);
			continue;
		}
		if (path == loc) {
			lcts.push_back(loc);
			continue;
		}
		if (path.size() > loc.size() &&
			path.compare(0, loc.size(), loc) == 0 &&
			path[loc.size()] == '/') {
			lcts.push_back(loc);
		}
	}
	if (lcts.empty()) {
		getSrvErrorPage(response, request.getSrvEntry(), NOT_FOUND);
		return Location();
	}
	std::vector<str>::iterator largest = std::max_element(lcts.begin(), lcts.end());
	request.setLocation(*largest);
	for(size_t i=0; i<locations.size(); ++i) {
		str loc = locations[i]._path;
		if (loc.size() > 1 && loc[loc.size() - 1] == '/')
			loc = loc.substr(0, loc.size() - 1);
		if(loc == *largest)
			return locations[i];
	}
	return Location();
}
