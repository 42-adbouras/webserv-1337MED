#pragma once

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
        const Status getStatus()const;
        void    setStatus(Status _status);

};