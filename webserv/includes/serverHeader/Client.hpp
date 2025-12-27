#pragma once

#include <ctime>
#include <cstring>
#include "SocketManager.hpp"
#include "../CGI.hpp"
#include "../request.hpp"
#include "../multipartParser.hpp"
#include "../ChunkedParser.hpp"

typedef	size_t	wsrv_timer_t;

enum    ClientState {   // Enum for Clients state only
    CS_NEW,
    CS_CGI_REQ,
    CS_READING,
    CS_READING_DONE,
    CS_WRITING,
    CS_WRITING_DONE,
    CS_DISCONNECT,
    CS_FATAL,
    CS_START_SEND
};

enum    Connection {
    NEW,
    CLOSED
};

enum    ClientCGIState{
    CCS_FAILLED,
    CCS_RUNNING,
    CCS_DONE
};

struct ReqInfo {
    ClientState			reqStatus;
    std::vector<char>	buffer;
};

struct  SendINfo /* */
{
    std::vector<char>	buff;
    ClientState 		resStatus;
    int         		fd;
	Connection			connectionState;
};

enum ParseState {
	PARSING_HEADERS,
	PARSING_BODY,
	REQUEST_COMPLETE
};

class   Client {
    private:
        int             	_fd;
        ClientState     	_clientState;
        // TIME-OUT attribute
        std::time_t     	_startTime;
        std::time_t     	_timeOut;
        std::time_t     	_remaining;
        // CGI attribute
        CGIContext      	_cgiContext;
        ClientCGIState  	_cltCgiState;
        // req/res attribute
        Request         	_request;
        Response        	_response;
        ClientState     	_requestType;
		str 				_leftover;
		size_t 				_expectedBodyLength;
		bool 				_isChunked;
        Client();
    public:
        int         		getFd() const;
        SendINfo    		_sendInfo;
        ReqInfo     		_reqInfo;
        serverBlockHint		_serverBlockHint;
        ClientState			getStatus() const;
        void        		setClientState(ClientState clientState);
        ~Client();
        Client(int fd, const serverBlockHint& server_block);
        // CGI Method
        bool            	_alreadyExec; /* cgi-script already executed or not */
        void        		setCgiContext(CGIContext& cgiContexty) ;
        CGIOutput   		_cgiOut;
        CGIProc        		_cgiProc;
        ClientCGIState		getCltCgiState() const;
        void        		setCltCgiState(ClientCGIState cltCgiState);
        const CGIContext&	getCgiContext(void) const;
		// Req/Res Method
		Request&    		getRequest();
		Response&   		getResponse();
		ParseState 			_state;
        void        		setRequest( Request& req );
        void        		setResponse( Response& res );
		str&				getLeftover( void );
		size_t				getExpectedBodyLength( void ) const;
		bool				getIsChunked( void ) const;
		void				setExpectedBodyLength( size_t lgth );
		void				setIsChunked( bool chunked );
		void				setLeftover( str& leftover );
		int					_uploadFd;
		str					_uploadPath;
		size_t				_uploadedBytes;
		MultipartParser 	_multipartParser;
		ChunkedParser		_chunkedParser;
		// TIME-OUT Methode
        void				setRemainingTime(wsrv_timer_t remaining);
        wsrv_timer_t		getRemainingTime(void) const;
        void        		setStartTime(std::time_t start);
        void        		setTimeOut(std::time_t timeout);
        std::time_t   		getTimeOut() const;
        std::time_t   		getStartTime(void) const;
};