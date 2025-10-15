
#pragma once

#include <exception>
#include <iostream>

class   ServerExcept : public std::exception {
    private:
        int _errno;
    public:
        ServerExcept(int err);
        const char* what() const _NOEXCEPT;
};