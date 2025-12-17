#pragma once

#include <fcntl.h>
// #include <iostream>
#include <unistd.h>
#include "response.hpp"

void    errPageNotFound( Response& resp, int errorCode );