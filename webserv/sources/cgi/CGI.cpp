/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/07 21:19:38 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/08 15:48:01 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/TypeDefs.hpp"
#include "../../includes/Response.hpp"
#include "../../includes/CGI.hpp"
#include <iostream>
#include <poll.h>
#include <sys/poll.h>
#include <unistd.h>
#include <signal.h>
// #include <cstring>

char**	setEnvironment( const str& path )
{
	std::vector<str>	envVect;

	envVect.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVect.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVect.push_back("REQUEST_METHOD=GET");
	envVect.push_back("SCRIPT_FILENAME=" + path);
	envVect.push_back("SERVER_SOFTWARE=ait-server/0.1");
	
	char** env = new char*[envVect.size() + 1];
	for (size_t i = 0; i < envVect.size(); ++i) {
		env[i] = strdup(envVect[i].c_str());
	}
	env[envVect.size()] = NULL;
	return (env);
}

CGIOutput	cgiHandle( str& path, str& ntrp, str& reqBody )
{
	CGIOutput	out;
	int			inPipe[2];
	int			outPipe[2];

	(void) reqBody;
	if (pipe(inPipe) < 0 || pipe(outPipe) < 0) {
		std::cerr << "pipe() facailed" << std::endl;
		return (CGIOutput(500, ""));
	}
	
	pid_t	pid = fork();
	if (pid < 0) {
		std::cerr << "fork() failed" << std::endl;
		close(outPipe[0]);
		close(outPipe[1]);
		close(inPipe[0]);
		close(inPipe[1]);
		return (CGIOutput(500, ""));
	}
	if (pid == 0) {
		char**	env = setEnvironment(path);

		char* av[3];
		av[0] = const_cast<char*>(ntrp.c_str());
		av[1] = const_cast<char*>(path.c_str());
		av[2] = NULL;

		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);
		close(outPipe[0]); close(outPipe[1]);
        close(inPipe[0]);  close(inPipe[1]);

		execve(ntrp.c_str(), av, env);
		std::cerr << "execve() failed" << std::endl;
		for (int i = 0; env[i]; ++i)
			free(env[i]);
		delete[] env;
		exit(EXIT_FAILURE);
	}
	close(inPipe[0]);
	close(outPipe[1]);
	if(!reqBody.empty()) {
		write(inPipe[1], reqBody.c_str(), reqBody.size());
	}
	close(inPipe[1]);

	struct pollfd	pFD;
	pFD.fd = outPipe[0];
	pFD.events = POLLIN;

	int ret = poll(&pFD, 1, 5000);
	if (ret == -1) {
		std::cerr << "poll() failed" << std::endl;
		kill(pid, SIGKILL);
		close(outPipe[0]);
		waitpid(pid, 0, 0);
		return (CGIOutput(500, ""));
	} else if (ret == 0) {
		std::cerr << "CGI timeout" << std::endl;
		kill(pid, SIGKILL);
		close(outPipe[0]);
		waitpid(pid, 0, 0);
		return (CGIOutput(504, ""));
	}
	
	char	buff[1024];
	int		byte;
	while ((byte = read(outPipe[0], buff, sizeof(buff))) > 0) {
		// buff[byte] = '\0';
		out._output.append(buff, byte);
	}
	close(outPipe[0]);
	int status = 0;
	waitpid(pid, &status, 0);
	return(out);
}
