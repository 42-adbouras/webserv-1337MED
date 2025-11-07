#pragma once

#include "../TypeDefs.hpp"
#include <ctime>

struct  Session;

class CookiesSessionManager
{
    private:
    public:
        std::map<std::string, Session>          _sessionTable;
        void        addSession(const std::string sessionId);
        std::string generateSessionId() const;
        std::string getCurrentId() ;
        CookiesSessionManager();
        ~CookiesSessionManager();
};

struct  Session {
    std::string name;
    bool        isLogedIn;
};