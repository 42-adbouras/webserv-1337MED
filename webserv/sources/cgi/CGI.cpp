/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/07 21:19:38 by adbouras          #+#    #+#             */
/*   Updated: 2025/11/14 16:25:30 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/CGI.hpp"
#include "../../includes/serverHeader/SocketManager.hpp"
#include <cstring>

str		joinQuery( const QueryMap& query )
{
	if (query.empty())
		return (str());
	std::ostringstream oss;
	for (QueryMap::const_iterator it = query.begin(); it != query.end(); ++it) {
		if (it != query.begin())
			oss << '&';
		oss << it->first << '=' << it->second;
	}
	return (oss.str());
}

str		convertHeader( const str& header )
{
	str	out;

	for (size_t i = 0;i < header.size(); ++i) {
		char c = header[i];
		if ( c == '-')
			out.push_back('_');
		else
			out.push_back(std::toupper(c));
	}
	return (out);
}

char**	buildEnv( const CGIContext& req )
{
	std::vector<str>	envVect;

	envVect.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVect.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVect.push_back("SERVER_SOFTWARE=ait-server/0.1");
	envVect.push_back("REQUEST_METHOD=" + req._method);
	envVect.push_back("SCRIPT_FILENAME=" + req._name);

	size_t	bSize = req._body.size();
	if (bSize > 0) {
		std::ostringstream oss;
		oss << bSize;
		envVect.push_back("CONTENT_LENGTH=" + oss.str());
	}

	// HeadersMap::const_iterator it = req._headers.find("Content-Type");
	// if (it != req._headers.end() && !it->second.empty())
	// envVect.push_back("CONTENT_TYPE="+ req._contenType);

	if (!req._query.empty())
		envVect.push_back("QUERY_STRING=" + joinQuery(req._query));

	if (!req._headers.empty()) {
		HeadersMap::const_iterator it = req._headers.begin();
		for (; it != req._headers.end(); ++it) {
			if (it->first == "Content-Lenght" || it->first == "Content-Type")
				continue;
			envVect.push_back("HTTP_" + convertHeader(it->first) + "=" + it->second);
		}
	}
	
	char**	env = new char*[envVect.size() + 1];
	
	for (size_t i = 0; i < envVect.size(); ++i) {
		const str& s = envVect[i];
		char* buf = new char[s.size() + 1];
		std::memcpy(buf, s.c_str(), s.size() + 1);
		env[i] = buf;
	}
	env[envVect.size()] = NULL;
	return (env);
}

CGIProc	cgiHandle( CGIContext& req )
{
	int			inPipe[2];
	int			outPipe[2];

	// try access first
	if (access(req._path.c_str(), R_OK | X_OK) < 0) {
		std::cerr << "access() denied" << std::endl;
		return (CGIProc(true, 403)); 
	}
	(void) req;
	if (pipe(inPipe) < 0 || pipe(outPipe) < 0) {
		std::cerr << "pipe() failed" << std::endl;
		return (CGIProc(true, 500));
	}

	pid_t	pid = fork();
	if (pid < 0) {
		std::cerr << "fork() failed" << std::endl;
		close(outPipe[0]); close(outPipe[1]);
		close(inPipe[0]); close(inPipe[1]);
		return (CGIProc(true, 500));
	}
	if (pid == 0) {
		char**	env = buildEnv(req);

		char* av[2];
		// av[0] = const_cast<char*>(req._ntrp.c_str());
		av[0] = const_cast<char*>(req._path.c_str());
		av[1] = NULL;

		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);

		close(outPipe[0]); close(outPipe[1]);
        close(inPipe[0]);  close(inPipe[1]);

		execve(av[0], av, env);
		std::cerr << "execve() failed" << std::endl;

		for (int i = 0; env[i]; ++i)
			delete[] env[i];
		delete[] env;
		exit(EXIT_FAILURE);
	}
	close(inPipe[0]);
	close(outPipe[1]);
	const str body = req._body; 
	if(!body.empty()) {
		write(inPipe[1], body.c_str(), body.size());
	}
	close(inPipe[1]);
	return (CGIProc(pid, outPipe[0], 200));
}

CGIOutput	readChild( const CGIProc& proc )
{
	CGIOutput	out;
	char		buff[2048];
	int			byte;

	while ((byte = read(proc._readPipe, buff, sizeof(buff))) > 0) {
		// buff[byte] = '\0';
		out._output.append(buff, byte);
	}
	close(proc._readPipe);
	int status = 0;
	waitpid(proc._childPid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		out._code = 500;
	return(out);
}
