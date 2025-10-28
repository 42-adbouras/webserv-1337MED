#include "../../includes/Request.hpp"
#include "../../includes/Response.hpp"

void notImplementedResponse( Response& response ) {
	response.setStatus(501);
	std::ifstream file("./www/defaultErrorPages/501.html");
	if (!file.is_open()) {
		throw Request::RequestException("501 error page not found!");
	} else {
		sstream buffer;
		buffer << file.rdbuf();
		response.setBody(buffer.str());
		response.addHeaders("Host", "localhost:8080");
		response.addHeaders("Content-Type", "text/html");
		response.addHeaders("Contenet-Length", iToString(buffer.str().length()));
	}
}

void URItooLongResponse( Response& response ) {
	response.setStatus(414);
	std::ifstream file("./www/defaultErrorPages/414.html");
	if (!file.is_open()) {
		throw Request::RequestException("414 error page not found!");
	} else {
		sstream buffer;
		buffer << file.rdbuf();
		response.setBody(buffer.str());
		response.addHeaders("Host", "localhost:8080");
		response.addHeaders("Content-Type", "text/html");
		response.addHeaders("Contenet-Length", iToString(buffer.str().length()));
	}
}