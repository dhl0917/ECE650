#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <stdio.h>
using namespace std;

int main(int argc, char *argv[])
{
  if(argc != 3){
    cout << "Please check the number of parameters."<<endl;
    return 1;
  }
  int status;
  int socket_server; // socket with server
  int socket_listen; // socket to listen the port
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

  socket_server = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);

  if (socket_server == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
  
  
  status = connect(socket_server, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
  freeaddrinfo(host_info_list);

  int connectedFlag = 1;
  send(socket_server, &connectedFlag, sizeof(connectedFlag), 0);

  int numberMessage[2];
  recv(socket_server,numberMessage,sizeof(numberMessage),0);
  int id = numberMessage[0];
  int num_players = numberMessage[1];
  cout<<"Connected as player "<<id<<" out of "<<num_players<<" total players"<<endl;


  bool syn = false;
  while(!syn){
    int signal;
    recv(socket_server,&signal,sizeof(signal),0); // Standby, waiting for orders from the server
    if(signal == 0){
      // All players connected and synchronized, break the loop
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
        cerr << "  (Player " << id << "," << atoi(port_to_listen) << ")" << endl;
        return -1;
      } //if
      assert(host_info_list->ai_next->ai_next==NULL);
      socket_listen = socket(host_info_list->ai_family, 
				 host_info_list->ai_socktype, 
				 host_info_list->ai_protocol);
      if (socket_listen == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (Player " << id << "," << atoi(port_to_listen) << ")" << endl;
        return -1;
      } //if

      int yes = 1;
      status = setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
      status = bind(socket_listen, host_info_list->ai_addr, host_info_list->ai_addrlen);
      if (status == -1) {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (Player " << id << "," << atoi(port_to_listen) << ")" << endl;
        return -1;
      } //if
      freeaddrinfo(host_info_list);

      status = listen(socket_listen, 100);
      if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl; 
        cerr << "  (" << hostname << "," << atoi(port_to_listen) << ")" << endl;
        return -1;
      } //if
      struct sockaddr_storage socket_addr_left;
      socklen_t socket_addr_len_left = sizeof(socket_addr_left);
      socket_left = accept(socket_listen, (struct sockaddr *)&socket_addr_left, &socket_addr_len_left);
      if (socket_left == -1) {
        cerr << "Error: cannot accept connection on the socket of player "<<id<< endl;
        return -1;
      } //if
      connectedFlag=0;
      recv(socket_left,&connectedFlag,sizeof(connectedFlag),0);
      if(connectedFlag==1){
        send(socket_server,&connectedFlag,sizeof(connectedFlag),0); // indicate the success to the server
      }
      else{
        cerr<<"Player "<<id<<"'s connection with the left is down"<<endl;
        return -1;
      }
    }
    else if(signal==2){
      struct addrinfo host_info;
      struct addrinfo *host_info_list;

      // Connect to the right neighbor
      struct sockaddr_storage socket_addr;
      recv(socket_server,&socket_addr,sizeof(struct sockaddr_storage),0);
      struct sockaddr_in *temp = (struct sockaddr_in *)&socket_addr;
      int port_to_connect;
      if(id == 0){
        port_to_connect = 5477 + num_players - 1;
      }
      else{
        port_to_connect = 5477 + id - 1;
      }
      char port_to_connect_str[50];
      assert(sprintf(port_to_connect_str,"%d",port_to_connect)>=0); // make sure converted successfully

      memset(&host_info, 0, sizeof(host_info));
      host_info.ai_family   = AF_UNSPEC;
      host_info.ai_socktype = SOCK_STREAM;

      status = getaddrinfo(inet_ntoa(temp->sin_addr), port_to_connect_str, &host_info, &host_info_list);
      if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << inet_ntoa(temp->sin_addr) << "," << port_to_connect << ")" << endl;
        return -1;
      } //if
      assert(host_info_list->ai_next==NULL);
      socket_right = socket(host_info_list->ai_family, 
            host_info_list->ai_socktype, 
            host_info_list->ai_protocol);

      if (socket_right == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << inet_ntoa(temp->sin_addr) << "," << port_to_connect << ")" << endl;
        return -1;
      } //if

      status = -1;
      while(status==-1){
	      status = connect(socket_right, host_info_list->ai_addr, host_info_list->ai_addrlen);
      }
      freeaddrinfo(host_info_list);
      connectedFlag = 1;
      send(socket_right, &connectedFlag, sizeof(connectedFlag), 0);
    }
  }
  
  int closedFlag=0;
  while(closedFlag==0){
    recv(socket_server,&closedFlag,sizeof(closedFlag),0);
  }
  

  close(socket_left);
  close(socket_right);
  close(socket_listen);
  close(socket_server);
  cout<<"Player "<<id<<" closed"<<endl;
  return 0;
}
