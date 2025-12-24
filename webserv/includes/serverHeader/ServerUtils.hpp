#pragma once

#include "../TypeDefs.hpp"

/* macro for output messages */

#define INFO "[ INFO ]"
#define CONNECTION "[ CONNECTION ]"
#define TIME_OUT    "[ TIME-OUT ]"
#define DISCONNECTION "[ DISCONNECTION ]"
#define LISTEN "[ LISTEN ]"
#define NOTICE "[ NOTICE ]"
#define SERVER "[ SERVER ]"
#define SOCKET_MANAGER "[ SOCKET_MANAGER ]"
#define WARNING "[ WARNING ]"
#include <iostream>

struct  CONSOLE {
    std::string message;
    CONSOLE() {};
    void    log(std::string tag, const std::string& mess, std::string color) {
        std::cout << BG_CYAN << tag << RESET << color << " —— " << mess << RESET << "\n" << std::endl;
    }
};

extern CONSOLE g_console;