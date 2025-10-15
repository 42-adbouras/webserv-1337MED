#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include "../../includes/request.hpp"
#include "../../includes/response.hpp"

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(8080);
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  std::cout << "Waiting for a client to connect...\n";
   while (true) {
        int client_socket = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t*)&client_addr_len);
        if (client_socket < 0) {
            std::cerr << "Failed to accept connection\n";
            continue;
        }
        std::cout << "Client connected\n";
        char  buffer[1024];
        ssize_t rByte = recv(client_socket, &buffer, sizeof(buffer), 0);
        if (rByte == 0)
        {
          std::cout << "Connection closed by the client!" << std::endl;
          close(client_socket);
          continue;
        }
        buffer[rByte] = '\0';

		std::cout << std::endl;
        std::cout << buffer << std::endl;

		requestHandler( buffer, client_socket );

        // std::string requestTarget(buffer);
        // requestTarget = requestTarget.substr(0, requestTarget.find_first_of('\r')); // expand Request line
        // size_t  start = requestTarget.find_first_of(' ') + 1;
        // requestTarget = requestTarget.substr(start, requestTarget.find_last_of(' ') - start); // expand Request target (URL PATH)
        // if (requestTarget == "/")
        //   send(client_socket, "HTTP/1.1 200 OK\r\n\r\n", 20, 0);
        //   else
        //   send(client_socket, "HTTP/1.1 404 Not Found\r\n\r\n", 27, 0);

        close(client_socket);
    }
  close(server_fd);
  return 0;
}