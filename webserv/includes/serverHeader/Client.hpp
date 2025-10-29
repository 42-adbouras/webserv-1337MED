#pragma once

#include <iostream>
#include <SocketManager.hpp>
#include "../TypeDefs.hpp"
#include "../request.hpp"

enum	Status {
	DISCONNECT,
	KEEP_ALIVE,
	NON
};

class   Client {
    private:
        int     _fd;
        Status  _status;
		Request _request;
        Client();
    public:
        serverBlockHint _serverBlockHint;
        Client(int fd, const serverBlockHint& server_block);
        ~Client();
        int     getFd() const;
        void    setFd(int fd);
        Status getStatus() const;
        void    setStatus(Status status);
		void	setRequest( Request req );
		Request& getRequest();

};