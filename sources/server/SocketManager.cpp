#include "../../includes/serverHeader/Server.hpp"
#include "../../includes/serverHeader/SocketManager.hpp"
#include "../../includes/serverHeader/ServerUtils.hpp"
#include "../../includes/serverHeader/Client.hpp"
#include "../../includes/serverHeader/CookiesSessionManager.hpp"
#include "../../includes/response.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
// #include "../CGI.hpp"
// #include "../../includes/Utils.hpp"

// CONSOLE g_console;

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
            tmp._serverBlockId = serverId;
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
        status = getaddrinfo(_tableOfListen[counter]._ip.c_str(), _tableOfListen[counter]._port.c_str(), &hints, &results);
        if (status != 0)
            throw std::runtime_error(gai_strerror(errno));
        int fd = socket(results->ai_family, results->ai_socktype, IPPROTO_TCP);
        if (fd == -1) {
            closeListenSockets();
            throw std::runtime_error(strerror(errno));
        }
        SocketManager::setNonBlocking(fd);
        _tableOfListen[counter]._fd = fd;
        _tableOfListen[counter].alreadyBinded = false;
        //-------------------------------
        // _tableOfListen[counter].addr = (reinterpret_cast<struct sockaddr*>(results->ai_addr));
        // freeaddrinfo(results);
        // bindSockets(counter);
        //-------------------------------------------
        std::memcpy(&_tableOfListen[counter].addr, results->ai_addr, results->ai_addrlen);
        _tableOfListen[counter].addr_len = results->ai_addrlen;
        freeaddrinfo(results);
        bindSockets(counter);
        /**
         * TODO:
         * checking leaks in this -> '_tableOfListen[counter].addr'
         * because it point to *results addrinfo struct
        */
    }
    
}

bool    SocketManager::checkIfAlreadyBinded(size_t index) const {
    for (size_t i = 0; i < index; i++)
    {
        if (_tableOfListen[i] == _tableOfListen[index])
            return true;
    }
    return false;
}

void    SocketManager::bindSockets(size_t counter) {
    int status = 0;
    int opValue = 1;

    if (!checkIfAlreadyBinded(counter) && (setsockopt(_tableOfListen[counter]._fd, SOL_SOCKET, SO_REUSEADDR, &opValue, sizeof(opValue)) != 0))
    {
        closeListenSockets();
        throw std::runtime_error(strerror(errno));
    }
    status = bind(_tableOfListen[counter]._fd, reinterpret_cast<struct sockaddr*>(&_tableOfListen[counter].addr), sizeof(struct sockaddr));
    if (status != 0 && errno == EADDRINUSE) {
        _tableOfListen[counter].alreadyBinded = true;
        close(_tableOfListen[counter]._fd);
        hanldVirtualHost(_tableOfListen[counter], counter);
        std::cout << "THIS: " << _tableOfListen[counter]._ip << ":" << _tableOfListen[counter]._port << ", already binded" << std::endl;
    }
    else  if (status == 0) {
        std::cout << "socket fd " << _tableOfListen[counter]._fd << ": binded successfull" << std::endl;
    }
}

void    SocketManager::hanldVirtualHost(TableOfListen& table, size_t index) {
    for (size_t i = 0; i < index; i++)
    {
        if (_tableOfListen[i] == table)
        {
            std::cout << "THE ACTUAL ONE: " << this->_tableOfListen[i]._ip << ":" << this->_tableOfListen[i]._port << std::endl;
            std::cout << RED << "IP:PORT that should be VIRTUAL => [ " << table._ip << ", " << table._port << " ] - [ " << _tableOfListen[i]._ip << ", " << _tableOfListen[i]._port << " ]" << RESET << std::endl;
            table._fd = _tableOfListen[i]._fd;
            break;
        }
    }
}
void    SocketManager::listenToPorts(void) {
    int status;
    errno = 0;
	for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        std::cout << GREEN << "<< " << _tableOfListen[i]._ip << ", " << _tableOfListen[i]._port << " >>" << RESET << std::endl;
        if (!_tableOfListen[i].alreadyBinded && (status = listen(_tableOfListen[i]._fd, SOMAXCONN)) == 0)
        {
            std::stringstream   oss;
            oss << str("Socket with `fd=") << _tableOfListen[i]._fd << "` listening!";
            g_console.log(LISTEN, oss.str(), CYAN);
        }
        else if(status != 0)
        {
            closeListenSockets();
            std::runtime_error(strerror(errno));    
        }  
        if (errno == EADDRINUSE)
        {
            std::cout << " ==================== IN THE LISTEN FUNCTION ===================" << std::endl;
            std::cout << strerror(errno) << std::endl;
            closeListenSockets();
            throw std::runtime_error(strerror(errno));
        }
    }
}

void    SocketManager::setListenEvent(std::vector<struct pollfd>& _pollfd) {
    size_t  count = 0;
    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (!_tableOfListen[i].alreadyBinded)
        {
            _pollfd[count].fd = _tableOfListen[i]._fd; // fill "struct pollfd" with listen socket FDs
            _pollfd[count].events = POLLIN;
            count++;
        }
    }
}

bool    SocketManager::checkForNewClients( std::vector<struct pollfd>& _pollfd, Server& _server ) {
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
            _server.addClients(Client(clientFd, detectServerBlock(_pollfd[i].fd)), _pollfd);
            SocketManager::setNonBlocking(clientFd);
            // std::stringstream   oss;
            oss.clear();
            oss.str("");
            oss << "New Client Connected with `fd=" << clientFd << "`!";
            g_console.log(CONNECTION, oss.str(), YELLOW);
        }
    }
    return true;
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
    g_console.log(SERVER, str("POLLING For Events..."), BG_GREEN);
    totalEvent = poll(pollFd.data(), pollFd.size(), static_cast<int>(coreTimeOut));
    if (totalEvent == 0 && cltSize > 0)
    {
        server.wsrv_timeout_closer(pollFd);
        std::cout << GREEN << "[ " << totalEvent << " ]" << RED<< " <<<<<<< EVENTS OCCURED ! >>>>>>>" << RESET << std::endl;
        return S_TIMEDOUT;
    }
    else if (totalEvent == -1)
    {
        closeListenSockets();
        server.closeClientConnection();
        std::cerr << BG_RED << "Poll() faill (-1)" << RESET << std::endl;
        throw   std::runtime_error(strerror(errno));
    }
    std::cout << GREEN << "[ " << totalEvent << " ]" << RED<< " <<<<<<< EVENTS OCCURED ! >>>>>>>" << RESET << std::endl;
    return NON;
}

void        SocketManager::handlErrCloses(std::vector<struct pollfd>& _pollfd, Server& server, size_t cltSize){
    size_t  clientStart = portCounter();
    std::stringstream   oss;
    (void)cltSize;
    for (size_t i = clientStart; i < server.getListOfClients().size(); i++)
    {
        if ( _pollfd[i].revents & (POLLHUP | POLLERR | POLLNVAL) )
        {
            // throw std::runtime_error("Client closed the connection unexpectedly");
            oss << "Client `fd:" << _pollfd[i].fd << "` Closed The Connection Unexpectedly.";
            g_console.log(WARNING, oss.str(), RED);
            server.handleDisconnect(i - portCounter(), _pollfd);
        }
    }
}

void    signalHandler(int sig) {
    std::cout << "\nSignal is: " << sig  << std::endl;
    exit(EXIT_SUCCESS);
}

void    SocketManager::runCoreLoop(void) {
    size_t                      cltStart = portCounter();
    CookiesSessionManager       sessionManager;
    Server                      _server(sessionManager, portCounter());
    std::vector<struct pollfd>  _pollfd(portCounter());
    std::stringstream   oss;

    setListenEvent(_pollfd);    // set listen socket to wait for POLLIN events
    std::vector<Client>&    _clients = _server.getListOfClients(); 
    signal(SIGINT, signalHandler);
    while (true)
    {
/* -------------------------------------------------------------------------------------
        if (portCounter() + _clients.size() < _pollfd.size())
        {
            for (size_t i = 0; i < _clients.size(); i++)
            {
                if (_clients[i]._cgiProc._childPid != -1)
                {
                    int results, status = 0;
                    results = waitpid(_clients[i]._cgiProc._childPid, &status, WNOHANG);
                    if (results == _clients[i]._cgiProc._childPid)
                    {
                        std::cout << CYAN << "Child status has been changed" << RESET << std::endl;
                    }
                    else if (results == 0)
                    {
                        std::cout << GREEN << "Child still running!" << RESET << std::endl;
                    }
                    else {
                        std::cerr << BG_RED << "Child process error" << std::endl;
                    }
                    // if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
                    //     out._code = 500;
                }
            }
        } 
---------------------------------------------------------------------------------------*/
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
            // cltScop = portCounter() + _clients.size();
 
            /*          Request Part       */
			
            if ( _pollfd[i].revents & POLLIN ) {
				// checking if req Done first;
                ClientState state = _server.readRequest(i - cltStart);
                if (state == CS_READING_DONE)
                {
					// requestHandler(_server.getListOfClients()[i - cltStart]);
                    _pollfd[i].revents = 0;
                    _pollfd[i].revents |= POLLOUT;
					
                }
                else if (state == CS_DISCONNECT || state == CS_FATAL)
                {
					std::stringstream   oss;
                    oss << "peer closed connection, `fd=" << _pollfd[i].fd << "`!";
                    g_console.log(NOTICE, oss.str(), RED);
                    _server.handleDisconnect(i - cltStart, _pollfd);
                    oss.clear();
                    oss.str("");
                    continue;
                }
                else if (state == CS_READING) /* means; it still reading ...*/
                    continue;
            }

            /*          Response Part       */

            if ( _pollfd[i].revents & POLLOUT )
            {
                g_console.log(SERVER, str("Response Handler"), BG_CYAN);
/*
                 * If User req CGI, Run CGI Script and wait for results next polling!
                 * ```!_clients[i-cltStart]._alreadyExec``` == Script Of CGI Already Running.
                 * So prevent to run it multiple-time!
                 */
                if (!_clients[i-cltStart]._alreadyExec && _clients[i-cltStart].getStatus() == CS_CGI_REQ) {
                    if (isCgiRequest(_pollfd, _clients[i - cltStart], i)) {
                        if (_clients[i-cltStart].getCltCgiState() == CCS_FAILLED) {
                            defErrorResponse(_clients[i-cltStart].getResponse(), _clients[i-cltStart]._cgiProc._statusCode);
                            _server.responsePart(i - cltStart);
                            _server.handleDisconnect(i - cltStart, _pollfd);
                        }
                        continue;
                    }
                }
                if (_server.getListOfClients()[i - cltStart]._sendInfo.resStatus == CS_START_SEND)
                {
                    
                    sendResponse(_server.getListOfClients()[i - cltStart]);

                }
                
                /**
                 * generate Response here & save it in `sendInfo` struct of client as chunks,
                 * when response finish, set `sendInfo.resStatus = CS_WRITING_DONE`
                 * 
                */

                _server.responsePart(i - cltStart);
                _pollfd[i].events &= ~POLLOUT;
                _pollfd[i].revents = 0;
            }
        }
	}
}

bool    SocketManager::isCgiRequest(std::vector<struct pollfd>& pollFd, Client& client, size_t index) {
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
        // g_console.log(SOCKET_MANAGER, , std::string color)
        std::cout << "/////////> CGI Pipe Added To Poll(); <//////////////" << std::endl;

        return true;
    }
    return  false;
}

/*
 * pipe = 16
 */

void    SocketManager::cgiEventsChecking(std::vector<Client>& clients, std::vector<struct pollfd>& pollFd, Server& srvr){
    size_t  cgiScop = portCounter() + clients.size();

    for (size_t i = cgiScop; i < pollFd.size(); i++)
    {
        if ( pollFd[i].revents & POLLIN )
        {
            std::cout << "Data To Read From CGI PIPE=" << pollFd[i].fd << std::endl;
            readFromCgi(clients, pollFd, srvr, &i);
            // pollFd[i].events |= POLLOUT;
            // g_console.log(INFO, str("Response In CGI wait for Send"), BG_GREEN);
        }
    }    
    
    // std::cout << "***********   Finish Reading  ******************" << std::endl;
}

void    SocketManager::readFromCgi(std::vector<Client>& clients, std::vector<struct pollfd>& pollFd, Server& srvr, size_t* coreIndex)
{
    /**
     * Loop over all clients to get The CGI source === (Client that request for CGI )
    */
    bool    found = false;
    for (size_t cg = 0; cg < clients.size(); cg++)
    {
        if (pollFd[(*coreIndex)].fd == clients[cg]._cgiProc._readPipe)
        {
            found = true;
		    std::cout << "Client fd=" << clients[cg].getFd() << " is ready to read from pipe=" << clients[cg]._cgiProc._readPipe <<std::endl;
            CGIOutput   out = readChild(clients[cg]);
            if (clients[cg].getCltCgiState() == CCS_DONE)
            {
                Response    res = clients[cg].getResponse();
                res.setStatus(out._code);
                res.addHeaders("Content-Type", "text/palin");
                res.addHeaders("Content-Length", iToString(res.getContentLength()));
                clients[cg].setResponse(res);
                srvr.responsePart(cg);
                g_console.log(INFO, str("********* CGI Response Sent ***********"), BG_BLUE);
                pollFd.erase(pollFd.begin() + (*coreIndex));
                // std::cout << BG_CYAN << "All Pollfd: \n[";
                // for (size_t i = 0; i < pollFd.size(); i++)
                // {
                //     std::cout << pollFd[i].fd << '|' << std::endl;
                // }
                // std::cout << ']' << RESET << std::endl;
                srvr.handleDisconnect(cg, pollFd);
            }
            else
                pollFd[*coreIndex].revents = 0;
        }
    }
    if (!found) {
        throw std::runtime_error("SocketManager::handleEvents: No socket found");
    }
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
        if (!_tableOfListen[i].alreadyBinded)
            close(_tableOfListen[i]._fd);
    }
    g_console.log(DISCONNECTION, "Listening Sockets Closed", RED);
    // std::cout << RED << "LISTENING SOCKETS CLOSED!" << RESET << std::endl;
}

//------ utils ------
size_t SocketManager::portCounter(void) const {
    size_t count = 0;
    for (size_t i = 0; i < _tableOfListen.size(); i++)
    {
        if (!_tableOfListen[i].alreadyBinded)
            count++;
    }
    return count;
}

