#include "CookiesSessionManager.hpp"

CookiesSessionManager::CookiesSessionManager() {}
CookiesSessionManager::~CookiesSessionManager() {
    _sessionTable.clear();
}

void    CookiesSessionManager::addSession(const std::string sessionId ) {
    std::pair<std::map<std::string, Session>::iterator, bool> ret;
    Session session = {"abc", false};
    ret = _sessionTable.insert(std::make_pair(sessionId, session));
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

std::string CookiesSessionManager::getCurrentId()  {
    std::map<std::string, Session>::iterator    it = _sessionTable.end();
    it--;
    return it->first;
}