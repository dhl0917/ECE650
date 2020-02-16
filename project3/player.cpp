#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
using namespace std;

int main(int argc, char *argv[])
{
  if(argc != 3){
    cout << "Please check the number of parameters."<<endl;
    return 1;
  }
  int status;
  int socket_fd; // socket with server
  int socket_left; // socket with the left neighbor
  int socket_right; // socket with the right neighbor
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = argv[1];
  const char *port     = argv[2];
  
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

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
  
  //  cout << "Connecting to " << hostname << " on port " << port << "..." << endl;
  
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  int connectedFlag = 1;
  send(socket_fd, &connectedFlag, sizeof(connectedFlag), 0);

  int numberMessage[2];
  recv(socket_fd,numberMessage,sizeof(numberMessage),0);
  int id = numberMessage[0];
  int num_players = numberMessage[1];
  cout<<"Connected as player "<<id<<" out of "<<num_players<<" total players"<<endl;


  bool syn = false;
  while(!syn){
    int signal;
    recv(socket_fd,&signal,sizeof(signal));
    if(signal==0){
      // Synchronized, break loop
      syn = true;
    }
    else if(signal == 1){
      // Build up the socket to listen
      char port_to_listen[50];
      assert(sprintf(port_to_listen,"%d",5477+id)>=0); // make sure converted successfully
      
      memset(&host_info, 0, sizeof(host_info));

      host_info.ai_family   = AF_UNSPEC;
      host_info.ai_socktype = SOCK_STREAM;
      host_info.ai_flags    = AI_PASSIVE;

      status = getaddrinfo(NULL, port_to_listen, &host_info, &host_info_list);
      if (status != 0) {
	cerr << "Error: cannot get address info for host" << endl;
	cerr << "  (" << hostname << "," << atoi(port_to_listen) << ")" << endl;
	return -1;
      } //if
      assert(host_info_list->ai_next->ai_next==NULL);
      int socket_listen = socket(host_info_list->ai_family, 
				 host_info_list->ai_socktype, 
				 host_info_list->ai_protocol);
      if (socket_listen == -1) {
	cerr << "Error: cannot create socket" << endl;
	cerr << "  (" << hostname << "," << atoi(port_to_listen) << ")" << endl;
	return -1;
      } //if

      int yes = 1;
      status = setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
      status = bind(socket_listen, host_info_list->ai_addr, host_info_list->ai_addrlen);
      if (status == -1) {
	cerr << "Error: cannot bind socket" << endl;
	cerr << "  (" << hostname << "," << atoi(port_to_listen) << ")" << endl;
	return -1;
      } //if

      status = listen(socket_listen, 100);
      if (status == -1) {
	cerr << "Error: cannot listen on socket" << endl; 
	cerr << "  (" << hostname << "," << atoi(port_to_listen) << ")" << endl;
	return -1;
      } //if
      struct sockaddr_storage socket_addr_left;
      socklen_t socket_addr_len_left = sizeof(socket_addr_left);
      socket_left = accept(socket_listen, (struct sockaddr *)&socket_addr_left, &socket_addr_len_left);
      
    }
  }



  
  
  
  int closedFlag=0;
  while(closedFlag==0){
    recv(socket_fd,&closedFlag,sizeof(closedFlag),0);
  }
  
  freeaddrinfo(host_info_list);
  close(socket_fd);
  cout<<"Player "<<id<<" closed"<<endl;
  return 0;
}
