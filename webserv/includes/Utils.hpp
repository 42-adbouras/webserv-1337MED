#pragma once

#include <fcntl.h>
#include <iostream>
#include <unistd.h>

int         setNonBlocking( int fd );
std::string iToString(int x);