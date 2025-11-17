#pragma once

#define G_TIME_OUT 10000 // globale time-out for poll() events in ms
#define CLIENT_HEADER_TIMEOUT 30 // second
#define CLIENT_BODY_TIMEOUT 60 // second
#define KEEPALIVE_TIMEOUT 75 // second

/* macro for output messages */

#define INFO "[ INFO ]"
#define CONNECTION "[ CONNECTION ]"
#define TIME_OUT    "[ TIME-OUT ]"
#define DISCONNECTION "[ DISCONNECTION ]"
#define LISTEN "[ LISTEN ]"
#define REQUEST "[ REQUEST ]"
#define NOTICE "[ NOTICE ]"
#define SERVER "[ SERVER ]"
#define SOCKET_MANAGER "[ SOCKET_MANAGER ]"
#define WARNING "[ WARNING ]"




struct  CONSOLE {
    std::string message;
    CONSOLE() {};
    void    log(std::string tag, const std::string& mess, std::string color) {
        std::cout << BG_CYAN << tag << RESET << color << " —— " << mess << RESET << "\n" << std::endl;
    }
};

// const std::string& toStrin

extern CONSOLE g_console;