#include "../includes/serverHeader/ServerExcept.hpp"
#include <cstring>

ServerExcept::ServerExcept(int err) : _errno(err) {}

const char* ServerExcept::what() const throw() {
    std::string str("Server: ");
    std::string mssg(str.append(strerror(_errno)));
    return mssg.c_str();
}