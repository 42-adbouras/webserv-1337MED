
#pragma once

#include <exception>
// #include <iostream>

class ServerExcept : public std::exception {
private:
        int _errno;
public:
        ServerExcept(int err);
        // Use noexcept for the exception specification (C++11+). It's the modern
        // equivalent of throw() and matches the std::exception::what() signature.
        const char* what() const throw();
};
