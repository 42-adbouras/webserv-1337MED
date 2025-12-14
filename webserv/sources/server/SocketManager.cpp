#include "../../includes/serverHeader/Server.hpp"
#include "../../includes/serverHeader/SocketManager.hpp"
#include "../../includes/serverHeader/ServerUtils.hpp"
#include "../../includes/serverHeader/Client.hpp"
#include "../../includes/serverHeader/CookiesSessionManager.hpp"
#include "../../includes/response.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/poll.h>

bool    g_run = true;

void    signalHandler(int sig) {
    std::cout << "\nSignal is: " << sig  << std::endl;
	g_run = false;
    return;
}

SocketManager::SocketManager(Data& config, std::vector<TableOfListen>& tableOfListen) : _config(&config), _tableOfListen(tableOfListen) {
    g_console.log(SOCKET_MANAGER, str("Start managing listening sockets..."), BG_GREEN);
}

void    SocketManager::setTableOfListen(std::vector<TableOfListen>& table) {
    std::set<std::pair<str, str> >::iterator    it;

    for (size_t serverId = 0; serverId < _config->_servers.size(); serverId++)
    {
        it = _config->_servers[serverId]._listen.begin();
        while (it != _config->_servers[serverId]._listen.end())
        {
            TableOfListen  tmp;
            tmp._fd = -1;  
            tmp.addr_len = 0;
            tmp._ip = it->first;
            tmp._port = it->second;
            tmp._serverName = _config->_servers[serverId]._serverName;
            tmp._serverBlockId = serverId; /* on each server block this ip:port come */
            tmp._interfaceState.alreadyBinded = false;
            tmp._interfaceState.fd = -1;
            table.push_back(tmp);
            it++;
        }
    }
}

int SocketManager::setNonBlocking(int fd) {
    return (fcntl(fd, F_SETFL, O_NONBLOCK));
}

void    SocketManager::initSockets(void) {
    struct  addrinfo    hints, *results;
    int status;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //tell getaddrinfo() that the returned address will be used
    //               for binding a server socket, not for connecting as a client.
    for (size_t counter = 0; counter < _tableOfListen.size(); counter++)
    {
        if (!_tableOfListen[counter]._interfaceState.alreadyBinded)
        {
            if ((status = getaddrinfo(_tableOfListen[counter]._ip.c_str(), _tableOfListen[counter]._port.c_str(), &hints, &results)) != 0)
                throw std::runtime_error(gai_strerror(errno));
            int fd = socket(results->ai_family, results->ai_socktype, IPPROTO_TCP);
            if (fd == -1) {
                closeListenSockets();
                throw std::runtime_error(strerror(errno));
            }
            SocketManager::setNonBlocking(fd);
            _tableOfListen[counter]._fd = fd;
            /* set same SOCKET_FD to each identique IP:PORT  */
            for (size_t S = 0; S < _tableOfListen.size(); S++)
            {
                if (_tableOfListen[S]._interfaceState.alreadyBinded && _tableOfListen[S] == _tableOfListen[counter])
                {
                    _tableOfListen[S]._interfaceState.fd = fd;
                    _tableOfListen[S]._fd = fd;
                }
            }
            /* set option for that socket */
            int addrYes = 1;
            setsockopt(_tableOfListen[counter]._fd, SOL_SOCKET, SO_REUSEADDR, &addrYes, sizeof(addrYes));
            std::memcpy(&_tableOfListen[counter].addr, results->ai_addr, results->ai_addrlen);
            _tableOfListen[counter].addr_len = results->ai_addrlen;
            freeaddrinfo(results);
            bindSockets(counter);
        }
    }
}

void    SocketManager::bindSockets(size_t counter) {
    int status = 0;

    status = bind(_tableOfListen[counter]._fd, reinterpret_cast<struct sockaddr*>(&_tableOfListen[counter].addr), sizeof(struct sockaddr));
    if (status != 0) {
        std::cerr << _tableOfListen[counter]._ip << ":" << _tableOfListen[counter]._port << ", " << counter;
        throw std::runtime_error(strerror(errno));
    }
    else  if (status == 0) {
        std::cout << GREEN << "IP:PORT->" << _tableOfListen[counter]._ip << ":" << _tableOfListen[counter]._port << RESET << std::endl;
        std::cout << "socket fd " << _tableOfListen[counter]._fd << ": binded successfull" << std::endl;
    }
}

void    SocketManager::listenToPorts(void) {
    int status;

	for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (_tableOfListen[i]._interfaceState.alreadyBinded)
            continue;
        std::cout << GREEN << "<< " << _tableOfListen[i]._ip << ", " << _tableOfListen[i]._port << " >>" << RESET << std::endl;
        if ((status = listen(_tableOfListen[i]._fd, SOMAXCONN)) == 0)
        {
            std::stringstream   oss;
            oss << str("Socket with `fd=") << _tableOfListen[i]._fd << "` listening!";
            g_console.log(LISTEN, oss.str(), CYAN);
        }
        else if(status != 0)
        {
            closeListenSockets();
            std::cout << _tableOfListen[i]._ip << _tableOfListen[i]._port << std::endl;
            std::runtime_error(strerror(errno));
        }
    }
}

void    SocketManager::setListenEvent(std::vector<struct pollfd>& _pollfd) {
    size_t  count = 0;

    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (!_tableOfListen[i]._interfaceState.alreadyBinded)
        {
            _pollfd[count].fd = _tableOfListen[i]._fd; // fill "struct pollfd" with listen socket FDs
            _pollfd[count].events = POLLIN;
            count++;
        }
    }
}

void    SocketManager::checkForNewClients( std::vector<struct pollfd>& _pollfd, Server& _server ) {
    int 	clientFd;

    for (size_t	i = 0; i < portCounter(); i++)
    {
        if (_pollfd[i].revents & POLLIN)
        {
            std::stringstream   oss;
            oss << "New Connection From Socket `fd=" << _pollfd[i].fd << '`';
            g_console.log(SOCKET_MANAGER, oss.str(), WHITE);

            if ((clientFd = accept(_pollfd[i].fd, NULL, NULL)) == -1)
            {
                std::cout << "IN ACCEPT FUNC" << std::endl;
                closeListenSockets();
                _server.closeClientConnection();
                throw std::runtime_error(strerror(errno));
            }
            int sigYes = 1;
            setsockopt(clientFd, SOL_SOCKET, SO_NOSIGPIPE, &sigYes, sizeof(sigYes));
            _server.addClients(Client(clientFd, detectServerBlock(_pollfd[i].fd)), _pollfd);
            SocketManager::setNonBlocking(clientFd);
            oss.clear();
            oss.str("");
            oss << "New Client Connected with `fd=" << clientFd << "`!";
            g_console.log(CONNECTION, oss.str(), YELLOW);
        }
    }
}

void    SocketManager::rmClientFromPoll(std::vector<struct pollfd>& _pollfd, size_t  cltSize) {
    for (size_t i = 0; i < cltSize; i++)
    {
        _pollfd.erase(_pollfd.begin() + portCounter() + i);
    }
}

Status  SocketManager::PollingForEvents(std::vector<struct pollfd>& pollFd, Server& server,size_t cltSize) {
    int 	        totalEvent;
    wsrv_timer_t    coreTimeOut;

    (cltSize == 0 ? coreTimeOut = -1 : coreTimeOut = server.wsrv_find_next_timeout()*1000);
    displayPOllList(pollFd);
    g_console.log(SERVER, str("POLLING For Events..."), BG_GREEN);
    totalEvent = poll(pollFd.data(), pollFd.size(), static_cast<int>(coreTimeOut));
    if (totalEvent == -1)
    {
        closeListenSockets();
        server.closeClientConnection();
        std::cerr << BG_RED << "Poll() faill (-1)" << RESET << std::endl;
        throw   std::runtime_error(strerror(errno));
    }
    server.wsrv_timeout_closer(pollFd); /* remove client that timed-out */
    std::cout << GREEN << "[ " << totalEvent << " ]" << RED<< " <<<<<<< EVENTS OCCURED ! >>>>>>>" << RESET << std::endl;
    return NON;
}

void    SocketManager::handlErrCloses(std::vector<struct pollfd>& _pollfd, Server& server, size_t cltSize){
    size_t  clientStart = portCounter();
    std::stringstream   oss;
    (void)cltSize;
    for (size_t i = clientStart; i < server.getListOfClients().size(); i++)
    {
        if ( _pollfd[i].revents & (POLLHUP | POLLERR | POLLNVAL) )
        {
            oss << "Client `fd:" << _pollfd[i].fd << "` Closed The Connection Unexpectedly.";
            g_console.log(WARNING, oss.str(), RED);
            server.handleDisconnect(i - portCounter(), _pollfd);
            i--;
        }
    }
}

void    SocketManager::runCoreLoop(void) {
    size_t                      cltStart = portCounter();
    CookiesSessionManager       sessionManager;
    Server                      _server(sessionManager, portCounter());
    std::vector<struct pollfd>  _pollfd(portCounter());
    std::stringstream   oss;

    setListenEvent(_pollfd);    // set listen socket to wait for POLLIN events
    std::vector<Client>&    _clients = _server.getListOfClients(); 
    signal(SIGPIPE, SIG_IGN);
    while (g_run)
    {
/* -------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------*/
        /**
         * TODO:
         * set apropriate response for user time-out ...
         * 
         */
        if (PollingForEvents(_pollfd, _server, _clients.size()) == S_TIMEDOUT)
            continue;
        handlErrCloses(_pollfd, _server, _clients.size()); // handle close/error from client-side.
/*
 ++++++++++++++++++  Check Listen Sockets For new Connect: ++++++++++++++++++
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/ 
        checkForNewClients(_pollfd, _server);
/*
 ++++++++++++++++++      Lists Available Ports & Users:     ++++++++++++++++++
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/ 
        // std::cout << BG_CYAN << "All Pollfd: \n[";
        // for (size_t i = 0; i < _pollfd.size(); i++)
        // {
        //     std::cout << _pollfd[i].fd << '|' << std::endl;
        // }
        // std::cout << ']' << RESET << std::endl;
/*
 ++++++++++++++++++            CGI Events Part:            ++++++++++++++++++
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/ 
        if (portCounter() + _clients.size() < _pollfd.size()) { /*ensure there is at least one cgi Running*/
            g_console.log(SERVER, str("CGI Events Parts"), BG_BLUE);
            cgiEventsChecking(_clients, _pollfd, _server);
            // std::cout << "<<<<<<<<<<<< After Cheking Pipe: >>>>>>>>>>>>>>>" << std::endl;

            // _server.responsePart(0);
            // _pollfd[_clients.size() -1].events &= ~POLLOUT;
            // _server.handleDisconnect(0, _pollfd);
        }
/*
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ++++++++++++++++++    Requests & Responses  From Users:   ++++++++++++++++++
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
*/
        // size_t  cltScop = portCounter() + _clients.size();
        for (size_t i = cltStart; i < ( portCounter() + _clients.size() ); i++) {

            /*     ***************     Request Part    ***************    */
			
            if ( _pollfd[i].revents & POLLIN ) {
                
                ClientState state = _server.readRequest(i - cltStart);
                _clients[i-cltStart].setStartTime(std::time(NULL));
                if (state == CS_READING_DONE)
                {
					std::cout << "request Done!" << std::endl;
                    _pollfd[i].events |= POLLOUT;
                    _pollfd[i].events &= ~POLLIN;
                }
                else if (state == CS_DISCONNECT || state == CS_FATAL)
                {
					std::stringstream   oss;
                    oss << "peer closed connection, `fd=" << _pollfd[i].fd << "`!";
                    g_console.log(NOTICE, oss.str(), RED);
                    _server.handleDisconnect(i - cltStart, _pollfd);
                    i--;
                    oss.clear();
                    oss.str("");
                    continue;
                }
                else if (state == CS_READING) /* means; it still reading request ...*/
                    continue;
            }

            /*    ***************      Response Part     ***************    */

            if ( _pollfd[i].revents & POLLOUT )
            {
                g_console.log(SERVER, str("Response Handler"), BG_CYAN);
/*
                 * If User req CGI, Run CGI Script and wait for results next polling!
                 * ```!_clients[i-cltStart]._alreadyExec``` == Script Of CGI Already Running.
                 * So prevent to run it multiple-time!
                 */
                Client&	client = _server.getListOfClients()[i - cltStart];
                if (client.getStatus() == CS_CGI_REQ) { /* ****** CGI Handler exec/response ****** */
                    if (!client._alreadyExec) /* run CGI-script for & add CGIproc{pipe-fd to pollfd, pid} to client-data */
                    {
                        /* set time-out for cgi-script */
                        std::cout << "<<<<<<<<< Is Cgi >>>>>>>>>" << std::endl;
                        if (isCgiRequest(_pollfd, _clients[i - cltStart], i)) {
                            client.setStartTime(std::time(NULL));
                            if (client.getCltCgiState() == CCS_FAILLED) {
                                CGI_errorResponse(_clients[i - cltStart], _clients[i - cltStart]._cgiProc._statusCode);
                                client.setTimeOut(CLIENT_HEADER_TIMEOUT); /* waiting for new request */
                                client.setClientState(CS_NEW);
                                _pollfd[i].events |= POLLIN;
                                _pollfd[i].events &= ~POLLOUT;
                            }
                            client.setTimeOut(CGI_TIME_OUT); /* wait for CGI-script to finish */
                            continue;
                        }
                    }
                    else if (client.getCltCgiState() == CCS_DONE) /** when CGI-script Done, we send appropriate response here. */
                    {
                        str buffer = client._cgiOut._output;
                        if (buffer.empty())
                        {
                            // nothing to send.
                            _pollfd[i].events |= POLLIN;
                            _pollfd[i].events &= ~POLLOUT;
                            client.setClientState(CS_NEW);
                            client._alreadyExec = false;
                            client.setResponse(Response());
                            g_console.log(INFO, str("********* CGI Response Sent ***********"), BG_BLUE);
                            std::cout << "Client that finish it's script is:" << _pollfd[i].fd << std::endl;
                            std::cout << BG_CYAN << "All Pollfd: \n[";
                            for (size_t k = 0; k < _pollfd.size(); k++)
                            {
                                std::cout << _pollfd[k].fd << '|' << std::endl;
                            }
                            std::cout << ']' << RESET << std::endl;
                            continue;
                        }

                        size_t  toSend = std::min<size_t>(buffer.size(), CGI_SEND_BUFFER);
                        std::cout << "ToSend is:" << toSend << std::endl;
                        ssize_t sendByte = send(client.getFd(), buffer.c_str(), toSend, 0);
                        if (sendByte == 0) {
                            _server.handleDisconnect(i - cltStart, _pollfd);
                            i--;

                        }
                        else if (sendByte < 0)
                        {
                            if (errno == EAGAIN || errno == EWOULDBLOCK)
                            {
                                // std::cout << "Try sending again..." << std::endl;
                                continue;
                            }
                            else
                                throw std::runtime_error(strerror(errno));
                        }
                        else
                        {
                            client._cgiOut._output.erase(client._cgiOut._output.begin(), client._cgiOut._output.begin() + sendByte);
                            // std::cout << "[INFO]: A chunk of data left" << std::endl;
                        }
                    }
                } /* **************************************************** */
                if (client.getStatus() != CS_CGI_REQ && client._sendInfo.resStatus != CS_WRITING_DONE)  /** Handle response for normal HTTP request */
				{
                    client.setStartTime(std::time(NULL));
					// std::cout << "------ Start Sending ------" << std::endl;
					sendResponse(client);
					size_t	dataLen = client._sendInfo.buff.size();
					const char* dataPtr = client._sendInfo.buff.data();

					ssize_t byte = send(_pollfd[i].fd, dataPtr, dataLen, 0);
					if (byte == 0)
					{
						// std::cout << "CLose connection " << std::endl;
						_server.handleDisconnect(i - cltStart, _pollfd);
                        i--;
						continue;
					}
					else if (byte < 0)
                        continue;
					else if (byte > 0)
						client._sendInfo.buff.erase(client._sendInfo.buff.begin(), client._sendInfo.buff.begin() + byte);
				}
				if (client._sendInfo.resStatus == CS_WRITING_DONE)
				{
                	// std::cout << "Finish writing" << std::endl;
                    if (client._sendInfo.connectionState == CLOSED) {
                        _server.handleDisconnect(i - cltStart, _pollfd);
                        i--;
                        continue;
                    }
                    if (client._sendInfo.fd != -1)
                    {
                        close(client._sendInfo.fd);
                        client._sendInfo.connectionState = NEW;
                        client._sendInfo.fd = -1;
                    }
                    client._sendInfo.buff.clear();
                    client.setClientState(CS_NEW);
                    client._reqInfo.reqStatus = CS_NEW;
					_pollfd[i].events |= POLLIN;
					_pollfd[i].events &= ~POLLOUT;
				}
            }
        }
	}
}

bool    SocketManager::isCgiRequest(std::vector<struct pollfd>& pollFd, Client& client, size_t index)
{
    std::cout << "REQUEST IS CGI" << std::endl;
    CGIProc proc;

    if (!client._alreadyExec)
    {
        proc = cgiHandle(client.getCgiContext(), &client._alreadyExec);
        if (proc._error)
        {
            std::cout << "<<<<<<<<<<<< CGI FAILLED HERE >>>>>>>>>>>>>>>" << std::endl;
            g_console.log(WARNING, str("Unexpected Error On CGI!"), BG_RED);
            client._cgiProc = proc;
            client.setCltCgiState(CCS_FAILLED);
            return true;
        }
        struct  pollfd  pollPipe;
        pollPipe.fd = proc._readPipe;
        pollPipe.events = POLLIN;
        pollPipe.revents = 0;
        pollFd.push_back(pollPipe);
        client._cgiProc = proc;
        pollFd[index].events &= ~POLLOUT;
        client.setCltCgiState(CCS_RUNNING);
        std::cout << BG_GREEN << "Client with fd<" << client.getFd() << "> has CGI Pipe<" << proc._readPipe << ">" << RESET << std::endl;
        std::cout << "/////////> CGI Pipe Added To Poll(); <//////////////" << std::endl;

        return true;
    }
    return  false;
}

void    SocketManager::cgiEventsChecking(std::vector<Client>& clients, std::vector<struct pollfd>& pollFd, Server& srvr){
    size_t  cgiScop = portCounter() + clients.size();

    for (size_t i = cgiScop; i < pollFd.size(); i++)
    {
        if ( pollFd[i].revents & (POLLIN | POLLHUP) )
        {
            std::cout << "Data To Read From CGI PIPE=" << pollFd[i].fd << std::endl;
            /**
             * read result from pipe;
             */
            Client& clinet = srvr.getClientReqCGI(pollFd[i].fd);
            if (pollFd[i].fd == clinet._cgiProc._readPipe)
            {
                std::cout << "Client fd=" << clinet.getFd() << " is ready to read from pipe=" << clinet._cgiProc._readPipe <<std::endl;
                readChild(clinet);
                /**
                 * check if Done to set The Client to POLLOUT & remove pipe from pollfd struct.
                 */
                if (clinet.getCltCgiState() == CCS_DONE || clinet.getCltCgiState() == CCS_FAILLED)
                {
                    for (size_t i = portCounter(); i < portCounter() + clients.size(); i++)
                    {
                        if (clinet.getFd() == pollFd[i].fd)
                        {
			                std::cout << "Client with fd=" << clinet.getFd() << ", Switched to POLLOUT" << std::endl;
                            pollFd[i].events |= POLLOUT;
                            pollFd[i].events &= ~POLLIN;
                        }
                    }
                    if (clinet._cgiProc._readPipe != -1) {
                        close(clinet._cgiProc._readPipe);
                        clinet._cgiProc._readPipe = -1;
                    }
                    pollFd.erase(pollFd.begin() + i); /* remove pipe fd from pollfd{} */
                    // clinet.setStartTime(std::time(NULL)); /* reset time-out for sending response */
                    // clinet.setTimeOut(CLIENT_BODY_TIMEOUT);/* ********************************* */
                    generate_CGI_Response(clinet); // generate headers for CGI
                    continue;
                }
            }
            else
                throw std::runtime_error("Can't found Client CGI");
        }
    }
    // std::cout << "***********   Finish Reading  ******************" << std::endl;
}

serverBlockHint    SocketManager::detectServerBlock(int sockFd) const {
    serverBlockHint   tmp;
    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (sockFd == _tableOfListen[i]._fd)
        {
            tmp.push_back(std::make_pair(&_tableOfListen[i],  &(_config->_servers[_tableOfListen[i]._serverBlockId])));
            std::cout << BLUE << "HII THERE! I GET THE BLOCKE SERVER." << std::endl;
            std::cout << "IP=" << _tableOfListen[i]._ip << ", PORT=" << _tableOfListen[i]._port << YELLOW << ", DOMAINE-NAME=[" << _tableOfListen[i]._serverName << "]" << RESET << std::endl;
        }
    }
    return tmp;
}

void    SocketManager::closeListenSockets(void) const {
    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (!_tableOfListen[i]._interfaceState.alreadyBinded)
            close(_tableOfListen[i]._fd);
    }
    g_console.log(DISCONNECTION, "Listening Sockets Closed", RED);
}

//------ utils ------
size_t SocketManager::portCounter(void) const {
    size_t count = 0;
    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (!_tableOfListen[i]._interfaceState.alreadyBinded)
            count++;
    }
    return count;
}

void    displayPOllList(const std::vector<pollfd>& list) {
    std::cout << CYAN << "List Of pllfd{}:" << RESET << std::endl;
    for (size_t i = 0; i < list.size(); i++)
    {
        std::cout << list[i].fd << "   ";
    }
    std::cout << GREEN << " -|" << RESET << std::endl;
}