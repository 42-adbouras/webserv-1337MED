#pragma once

#include "../TypeDefs.hpp"
#include <ctime>
#include "./Client.hpp"

#define ROOT_LOGIN "/login"

struct  Session;

class CookiesSessionManager
{
    private:
        std::vector<std::pair<std::string, Session> >          _sessionTable;
    public:

        std::vector<std::pair<std::string, Session> >&  getSessionTable( void );
        void        addSession(const std::string sessionId);
        std::string   getParamValue( const std::string& cookies, const std::string& key);
        std::string generateSessionId() const;
        int         findSessionIfExist( const std::string& id ) const;
        void        setCookies(Client& client, const str& id, size_t counterLog );
        size_t      getLogCounter( const str& id ) const;
        void        displayAllSession() const;
        void        createNewSession(CookiesSessionManager& sessionManager, Client& client);
        CookiesSessionManager();
        ~CookiesSessionManager();
};

struct  Session {
    // std::string     name;
    size_t          logCounter;
};

void    userLoginHandl(Client& client, CookiesSessionManager& sessionManager);