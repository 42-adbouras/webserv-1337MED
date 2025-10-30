#pragma once

#include <ctime>
#include <SocketManager.hpp>
#include "ServerUtils.hpp"
#include "../TypeDefs.hpp"
#include "../request.hpp"

enum	Status {
	DISCONNECT,
	KEEP_ALIVE,
	NON
};

enum    ClientState {
    CS_NEW,
    CS_READING,
    CS_READING_DONE,
    CS_WRITING,
    CS_WRITING_DONE,
    CS_KEEPALIVE,
    CS_DISCONNECT
    // CS_CGI_PROCESSING // FOR CGI REQUEST
};

class   Client {
    private:
        int             _fd;
        ClientState     _clientState;
		Request         _request;
        std::time_t     _startTimeOut;
    
        Client();
    public:
        serverBlockHint _serverBlockHint;
        Client(int fd, const serverBlockHint& server_block);
        ~Client();
        int         getFd() const;
        void        setFd(int fd);
        ClientState getStatus() const;
        void        setTimeOut(std::time_t current);
        void        setClientState(ClientState clientState);
		void	    setRequest( Request req );
		Request&    getRequest();

};