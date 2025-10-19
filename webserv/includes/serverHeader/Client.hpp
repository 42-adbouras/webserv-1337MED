#pragma once

#include <iostream>

enum	Status {
	DISCONNECT,
	KEEP_ALIVE,
	NON
};


class   Client {
    private:
        int     _fd;
        Status  _status;
        Client();
    public:
        Client(int fd);
        ~Client();
        int     getFd() const;
        void    setFd(int fd);
        Status getStatus() const;
        void    setStatus(Status status);

};