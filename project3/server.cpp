#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
using namespace std;

int main(int argc, char *argv[])
{
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port     = "4444";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl; 
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  cout << "Waiting for connection on port " << port << endl;
  
  

  
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int socket_array[2];
  int client_connection_fd;
  int i = 0;
  while(i<2){
    //client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    socket_array[i] = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (socket_array[i] == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    } //if
    i += 1;
  }
  //const char *message = "0";
  int number = 0;
  char message[50];
  sprintf(message,"potato:%d",number);
  send(socket_array[0], &message, strlen(message), 0);

  number = 1;
  sprintf(message,"potato:%d",number);
  send(socket_array[1], &message, strlen(message), 0);
  
  char buffer[512];
  recv(socket_array[0], buffer, 10, 0);
  buffer[10] = 0;

  cout << "Server received: " << buffer << endl;

  char buffer2[512];
  recv(socket_array[1], buffer2, 10, 0);
  buffer[10] = 0;

  cout << "Server received: " << buffer2 << endl;

  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
