#pragma once

#include <ctime>
// #include <SocketManager.hpp>
#include <cstring>
#include "SocketManager.hpp"
// #include "ServerUtils.hpp"
#include "../CGI.hpp"
// #include "../CGI.hpp"
#include "../request.hpp"
// class Request;

enum    ClientState {   // Enum for Clients state only
    CS_NEW,
    CS_CGI_REQ,
    CS_TIMEDOUT,
    CS_OLD,
    CS_READING,
    CS_READING_DONE,
    CS_WRITING,
    CS_WRITING_DONE,
    CS_KEEPALIVE,
    CS_DISCONNECT,
    CS_NORM_REQ
    // CS_CGI_PROCESSING // FOR CGI REQUEST
};

enum    ClientCGIState{
    CCS_FAILLED,
    CCS_RUNNING,
    CCS_DONE
};

class   Client {
    private:
        int             _fd;
        ClientState     _clientState;
        ClientCGIState  _cltCgiState;
        Request         _request;
        Response        _response;
        std::time_t     _startTime;
        std::time_t     _timeOut;
        std::time_t     _remaining; // time-out
        // CGI
        ClientState     _requestType;
        CGIContext      _cgiContext;
        // CGIContext      _cgiContext;
        Client();
    public:
        void    setCgiContext(CGIContext& cgiContexty) ;
        serverBlockHint _serverBlockHint;
        CGIProc         _cgiProc;
        bool            _alreadyExec;
        // CGIProc         _cgiProc;
        Client(int fd, const serverBlockHint& server_block);
        ~Client();
        int         getFd() const;
        void        setFd(int fd);
        const CGIContext&  getCgiContext(void) const;
		Request&    getRequest();
		Response&   getResponse();
        ClientState getStatus() const;
        void        setClientState(ClientState clientState);
        void        setRequest( Request req );
        void        setResponse( Response res );
        void        setCltCgiState(ClientCGIState cltCgiState);
        ClientCGIState  getCltCgiState() const;
        // Time-Out methode handler
        
        void        setRemainingTime(wsrv_timer_t remaining);
        wsrv_timer_t getRemainingTime(void) const;
        void        setStartTime(std::time_t start);
        void        setTimeOut(std::time_t timeout);
        std::time_t   getTimeOut() const;
        std::time_t   getStartTime(void) const;

};
