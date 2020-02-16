#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <assert.h>
using namespace std;

int main(int argc, char *argv[])
{
   if(argc != 4){
    cout << "Please check the number of parameters."<<endl;
    return 1;
  }
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port     = argv[1];
  int num_players = atoi(argv[2]);
  int num_hops = atoi(argv[3]);
  if(num_players<=1){
    cout<<"num_players must be greater than 1."<<endl;
    return 1;
  }
  if(num_hops<0||num_hops>512){
    cout<<"num_hops must in the range [0,512]."<<endl;
    return 1;
  }
    
  cout<<"Potato Ringmaster"<<endl;
  cout<<"Players = "<<num_players<<endl;
  cout<<"Hops = "<<num_hops<<endl;

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
  assert(host_info_list->ai_next->ai_next == NULL);
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


  // Three lists to store information for each player
  int socket_array[num_players];
  struct sockaddr_storage socket_addr_list[num_players];
  socklen_t socket_addr_len_list[num_players];

  int playerCounter = 0;
  while(playerCounter<num_players){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    socket_array[playerCounter] = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    
    if (socket_array[playerCounter] == -1) {
      cerr << "Error: cannot accept connection on the socket from player "<<playerCounter<< endl;
      return -1;
    } //if

    struct sockaddr_in *temp = (struct sockaddr_in *)&socket_addr;
    cout<<inet_ntoa(temp->sin_addr)<<endl;
    cout<<temp->sin_port<<endl;
    
    socket_addr_list[playerCounter] = socket_addr;
    socket_addr_len_list[playerCounter] = socket_addr_len;

    int connectedFlag=0;
    recv(socket_array[playerCounter],&connectedFlag,sizeof(connectedFlag),0);
    if(connectedFlag==1){
      cout<<"Player "<<playerCounter<<" is ready to play"<<endl;
      int numberMessage[2];
      numberMessage[0] = playerCounter;
      numberMessage[1] = num_players;
      send(socket_array[playerCounter],&numberMessage,sizeof(numberMessage),0);
    }
    else{
      cerr<<"Player "<<playerCounter<<" is down"<<endl;
      return -1;
    }
    playerCounter += 1;
  }
  // Send target IP addresses
  for(int i=0;i<num_players;i++){
    int to_listen = 1;
    int to_connect = 2;
    send(socket_array[i],&to_listen,sizeof(to_listen),0);
    
    if(i+1!=num_players){
      send(socket_array[i+1],&to_connect,sizeof(to_connect),0);
      send(socket_array[i+1],&socket_addr_list[i],socket_addr_len_list[i],0);
    }
    else{
      send(socket_array[0],&to_connect,sizeof(to_connect),0);
      send(socket_array[0],&socket_addr_list[i],socket_addr_len_list[i],0);
    }
    int successFlag = 0;
    recv(socket_array[i],&successFlag,sizeof(successFlag),0);
    if(successFlag != 1){
      cerr<<"Building the connection between Player "<<i<< " and its left neighbor failed."<<endl;
      return -1;
    }
  }

  for(int i=0;i<num_players;i++){
    int closedFlag=1;
    send(socket_array[i], &closedFlag, sizeof(closedFlag), 0);
    close(socket_array[i]);
  }

  
  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
