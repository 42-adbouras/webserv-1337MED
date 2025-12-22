#include "CookiesSessionManager.hpp"

CookiesSessionManager::CookiesSessionManager() {}
CookiesSessionManager::~CookiesSessionManager() {
    _sessionTable.clear();
}

void    CookiesSessionManager::addSession(const std::string sessionId ) {
    Session newSession = { 1 };
    _sessionTable.push_back(std::make_pair(sessionId, newSession));
}

std::vector<std::pair<std::string, Session> >&  CookiesSessionManager::getSessionTable( void ) {
    return _sessionTable;
}

std::string CookiesSessionManager::generateSessionId() const {
    srand(static_cast<unsigned int>(std::time(NULL)));
    size_t  randomNumber = rand();
    char    randomChar = 'A' + (rand() % 26);
    std::stringstream   ss;

    ss << randomNumber;
    std::string id(1, randomChar);
    id += ss.str();
    return id;
}

size_t  CookiesSessionManager::getLogCounter( const str& id ) const{
    std::vector<std::pair<std::string, Session> >::const_iterator  it;

    for (it = _sessionTable.begin(); it != _sessionTable.end(); it++) {
        if (it->first == id)
        {
            return it->second.logCounter;
        }
    }
    std::cout << BG_RED << "Session Id u searching for it is invalid!" << RESET << std::endl;
    return 0;
}

std::string   CookiesSessionManager::getParamValue( const std::string& cookies, const std::string& key) {
    size_t  pos;
    size_t  end;

    pos = cookies.find(key);
    if (key == str("id")) {
        end = cookies.find("&");
        pos += 3;
        end -= pos;
    }
    else {
        pos += 11;
        end = cookies.size();
    }
    str res = cookies.substr(pos, end);
    return res;
}

int CookiesSessionManager::findSessionIfExist( const std::string& id ) const { /* return sessionTable Index for the current user  */
    std::vector<std::pair<std::string, Session> >::const_iterator   it;
    int index = 0;

    for (it = _sessionTable.begin(); it != _sessionTable.end(); it++)
    {
        if (it->first == id)
            return index;
        index++;
    }
    return -1;
}

void    userLoginHandl(Client& client, CookiesSessionManager& sessionManager) { /* handle session & cookies */
	// std::cout << BLUE << "path before: " << client.getRequest().getPath() << RESET << std::endl;
    size_t  pos;
    if ( (pos = client.getRequest().getPath().find(ROOT_LOGIN)) == std::string::npos )
    {
        return;
    }
	// std::cout << BG_BLUE << "path alter: " << client.getRequest().getPath() << RESET << std::endl;
    sessionManager.displayAllSession();
    const HeadersMap::const_iterator    it = client.getRequest().getHeaders().find(str("Cookie")); /* check if browser set Cookie */
    if (it != client.getRequest().getHeaders().end()) /* browser send Cookie; so this User already have a Session */
    {
        std::cout << "<<<<<<<<< Cookies exist >>>>>>>>>" << std::endl;
        int index;
        
        index = sessionManager.findSessionIfExist(sessionManager.getParamValue(it->second, str("id"))); /* get idndex of the current User in SessionTable{} */
        if (index != -1)
        {
            std::cout << BG_BLUE << "Session ID exist in server DataBase: " << sessionManager.getParamValue(it->second, str("id")) << RESET << std::endl;
            sessionManager.getSessionTable()[index].second.logCounter += 1; /* increment LogCounter for that user */
            /* Update the cookies for that user (same ID, but logCounter updated by +1) */
            sessionManager.setCookies(client, sessionManager.getParamValue(it->second, str("id")), sessionManager.getSessionTable()[index].second.logCounter);
            std::cout << GREEN << "Cookie i get: " << it->second << RESET << std::endl;
        }
        else { /* browser sent Cookie Header, but server restart so all Session removed */
            std::cout << BG_RED << "Session ID dosen't exist in the server DB" << std::endl;
            sessionManager.createNewSession(sessionManager, client);
        }
    }
    else /* No Cookie Header sent by browser */
        sessionManager.createNewSession(sessionManager, client);
}

void    CookiesSessionManager::createNewSession(CookiesSessionManager& sessionManager, Client& client) {
    const std::string   sessionId = sessionManager.generateSessionId();
    size_t  logCounter;

    sessionManager.addSession(sessionId);
    logCounter = sessionManager.getLogCounter(sessionId);
    sessionManager.setCookies(client, sessionId, logCounter);
    std::cout << BG_GREEN << "Session Id that i set: " << sessionId << RESET << std::endl;
}

void    CookiesSessionManager::displayAllSession() const {
    std::cout << BG_RED << "SHOW TABLES:" << RESET << std::endl;
    for(size_t i = 0; i < _sessionTable.size(); i++) {
        std::cout << GREEN << "Session " << i + 1 << ": " << _sessionTable[i].first << RESET << std::endl;
    }
}

void    CookiesSessionManager::setCookies(Client& client, const str& id, size_t counterLog) {
    str cookies;

    cookies = str("id=") + id + str("&counterLog=") + iToString(counterLog);
    client.getResponse().addHeaders(str("Set-Cookie"), cookies);
}