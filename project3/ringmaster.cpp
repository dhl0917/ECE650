#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>

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
  

  
  

  for(int i=0;i<num_players;i++){
    int closedFlag=1;
    send(socket_array[i], &closedFlag, sizeof(closedFlag), 0);
  }
  
  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
