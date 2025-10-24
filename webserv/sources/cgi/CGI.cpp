/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adbouras <adbouras@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/07 21:19:38 by adbouras          #+#    #+#             */
/*   Updated: 2025/10/22 14:53:32 by adbouras         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/CGI.hpp"
#include <stdlib.h> // remove it pleaaaaaaaazzzzzee!!!!!!!!
char**	buildEnv( Request& req )
{
	std::vector<str>	envVect;

	envVect.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVect.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVect.push_back("SERVER_SOFTWARE=ait-server/0.1");
	envVect.push_back("REQUEST_METHOD=" + req.getMethod());
	envVect.push_back("SCRIPT_FILENAME=" + req.getPath());

	size_t	bSize = req.getBody().size();
	if (bSize > 0) {
		std::ostringstream oss;
		oss << bSize;
		envVect.push_back("CONTENT_LENGTH=" + oss.str());
	}

	char**	env = new char*[envVect.size() + 1];
	
	for (size_t i = 0; i < envVect.size(); ++i)
		env[i] = strdup(envVect[i].c_str());
	env[envVect.size()] = NULL;
	return (env);
}

str		toUpper( const str& header )
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

CGIOutput	cgiHandle( Request& req )
{
	CGIOutput	out;
	int			inPipe[2];
	int			outPipe[2];

	(void) req;
	if (pipe(inPipe) < 0 || pipe(outPipe) < 0) {
		std::cerr << "pipe() failed" << std::endl;
		return (CGIOutput(500, ""));
	}
	
	pid_t	pid = fork();
	if (pid < 0) {
		std::cerr << "fork() failed" << std::endl;
		close(outPipe[0]); close(outPipe[1]);
		close(inPipe[0]); close(inPipe[1]);
		return (CGIOutput(500, ""));
	}
	if (pid == 0) {
		char**	env = buildEnv(req);

		char* av[3];
		av[0] = const_cast<char*>(req.getNTRP().c_str());
		av[1] = const_cast<char*>(req.getPath().c_str());
		av[2] = NULL;

		dup2(inPipe[0], STDIN_FILENO);
		dup2(outPipe[1], STDOUT_FILENO);
		close(outPipe[0]); close(outPipe[1]);
        close(inPipe[0]);  close(inPipe[1]);

		execve(req.getNTRP().c_str(), av, env);
		std::cerr << "execve() failed" << std::endl;
		for (int i = 0; env[i]; ++i)
			free(env[i]);
		delete[] env;
		exit(EXIT_FAILURE);
	}
	close(inPipe[0]);
	close(outPipe[1]);
	const str body = req.getBody(); 
	if(!body.empty()) {
		write(inPipe[1], body.c_str(), body.size());
	}
	close(inPipe[1]);

	char	buff[1024];
	int		byte;
	while ((byte = read(outPipe[0], buff, sizeof(buff))) > 0) {
		// buff[byte] = '\0';
		out._output.append(buff, byte);
	}
	close(outPipe[0]);
	int status = 0;
	waitpid(pid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		out._code = 500;
	return(out);
}
