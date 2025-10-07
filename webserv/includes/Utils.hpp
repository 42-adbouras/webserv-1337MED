#pragma once

#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sstream>

int         setNonBlocking( int fd );
std::string iToString(int x);