#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <assert.h>
#include "potato.h"

using namespace std;

int safe_send(int socket_fd,const void * ptr,size_t size,int flags){
  int bytes = 0;
  int retval;
  while(bytes!=size){
    retval = send(socket_fd,ptr,size,flags);
    if(retval == -1){
      cerr<<"Sending failed"<<endl;
      return 1; // Returning 1 to indicate failure
    }
    bytes += retval;
  }
  return 0; // Returning 0 to indicate success
}


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

  // listen to the port
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

  // Connect with every single player
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
    // cout<<inet_ntoa(temp->sin_addr)<<endl;
    // cout<<temp->sin_port<<endl;
    
    socket_addr_list[playerCounter] = socket_addr;
    socket_addr_len_list[playerCounter] = socket_addr_len;

    int connectedFlag=0;
    //recv(socket_array[playerCounter],&connectedFlag,sizeof(connectedFlag),0);
    assert(recv(socket_array[playerCounter],&connectedFlag,sizeof(connectedFlag),MSG_WAITALL)==sizeof(connectedFlag));
    if(connectedFlag==1){
      cout<<"Player "<<playerCounter<<" is ready to play"<<endl;
      int numberMessage[2];
      numberMessage[0] = playerCounter;
      numberMessage[1] = num_players;
      //send(socket_array[playerCounter],&numberMessage,sizeof(numberMessage),0);
      if(safe_send(socket_array[playerCounter],&numberMessage,sizeof(numberMessage),0)){return -1;}
    }
    else{
      cerr<<"Player "<<playerCounter<<" is down"<<endl;
      return -1;
    }
    playerCounter += 1;
  }

  // Send target IP addresses and connect all players
  for(int i=0;i<num_players;i++){
    int to_listen = 1;
    int to_connect = 2;
    if(safe_send(socket_array[i],&to_listen,sizeof(to_listen),0)){return -1;}
    //send(socket_array[i],&to_listen,sizeof(to_listen),0);
    
    if(i+1!=num_players){
      //send(socket_array[i+1],&to_connect,sizeof(to_connect),0);
      //send(socket_array[i+1],&socket_addr_list[i],socket_addr_len_list[i],0);
      if(safe_send(socket_array[i+1],&to_connect,sizeof(to_connect),0)){return -1;}
      if(safe_send(socket_array[i+1],&socket_addr_list[i],socket_addr_len_list[i],0)){return -1;}
    }
    else{
      //send(socket_array[0],&to_connect,sizeof(to_connect),0);
      //send(socket_array[0],&socket_addr_list[i],socket_addr_len_list[i],0);
      if(safe_send(socket_array[0],&to_connect,sizeof(to_connect),0)){return -1;}
      if(safe_send(socket_array[0],&socket_addr_list[i],socket_addr_len_list[i],0)){return -1;}
    }
    int successFlag = 0;
    //recv(socket_array[i],&successFlag,sizeof(successFlag),0);
    assert(recv(socket_array[i],&successFlag,sizeof(successFlag),MSG_WAITALL)==sizeof(successFlag));
    if(successFlag != 1){
      cerr<<"Building the connection between Player "<<i<< " and its left neighbor failed."<<endl;
      return -1;
    }
  }

  // Tell all players that all are synchronized
  for(int i=0;i<num_players;i++){
    int synFlag=0;
    //send(socket_array[i], &synFlag, sizeof(synFlag), 0);
    if(safe_send(socket_array[i], &synFlag, sizeof(synFlag), 0)){return -1;}
  }
  
  // Initialize the potato
  potato_t potato;
  memset(&potato, 0, sizeof(potato));
  potato.num_hops = num_hops;
  potato.index = -1;
  potato.GAMEOVER = 0;

  if(potato.num_hops == 0){
    // Tell all players GAMEOVER
    potato.GAMEOVER = 1;
    for(int i=0;i<num_players;i++){
      if(safe_send(socket_array[i],&potato,sizeof(potato),0)){return -1;}
    }

    // Synchronize all players to gameover state
    for(int i=0;i<num_players;i++){
      int gameoverFlag = 0;
      assert(recv(socket_array[i],&gameoverFlag,sizeof(gameoverFlag),MSG_WAITALL)==sizeof(gameoverFlag));
      assert(gameoverFlag==1);
    }

    // Tell all players to close sockets
    for(int i=0;i<num_players;i++){
      int closedFlag=1;
      //send(socket_array[i], &closedFlag, sizeof(closedFlag), 0);
      if(safe_send(socket_array[i], &closedFlag, sizeof(closedFlag), 0)){return -1;}
      close(socket_array[i]);
    }

    
    freeaddrinfo(host_info_list);
    close(socket_fd);

    return 0;
  }

  srand((unsigned int)time(NULL)+num_players);
  int random_start = rand() % num_players;

  // Send the first potato
  //send(socket_array[random_start],&potato,sizeof(potato),0);
  if(safe_send(socket_array[random_start],&potato,sizeof(potato),0)){return -1;}
  cout<<"Ready to start the game, sending potato to player "<<random_start<<endl;


  // Select where the potato coming back
  fd_set rfds;
  int retval;
  int max_fdid=-1;

  FD_ZERO(&rfds);
  for(int i=0;i<num_players;i++){
    FD_SET(socket_array[i], &rfds);
    max_fdid = (socket_array[i]>max_fdid)?socket_array[i]:max_fdid;
  }
  
  retval = select(max_fdid+1, &rfds, NULL, NULL, NULL);
  if (retval == -1){
      cerr<<"select()"<<endl;
  }

  for(int i=0;i<num_players;i++){
    if(FD_ISSET(socket_array[i], &rfds)){
      //recv(socket_array[i],&potato,sizeof(potato),0);
      assert(recv(socket_array[i],&potato,sizeof(potato),MSG_WAITALL)==sizeof(potato));
      break;
    }
  }
  FD_ZERO(&rfds);

  // Receive from it and print the results
  cout<<"Trace of potato:"<<endl;
  for(int i=0;i<num_hops;i++){
    if(i==0){
      cout<<potato.trace[i];
    }
    else{
      cout<<","<<potato.trace[i];
    }
  }
  cout<<endl;

  // Tell all players GAMEOVER
  potato.GAMEOVER = 1;
  for(int i=0;i<num_players;i++){
    if(safe_send(socket_array[i],&potato,sizeof(potato),0)){return -1;}
  }

  // Synchronize all players to gameover state
  for(int i=0;i<num_players;i++){
    int gameoverFlag = 0;
    assert(recv(socket_array[i],&gameoverFlag,sizeof(gameoverFlag),MSG_WAITALL)==sizeof(gameoverFlag));
    assert(gameoverFlag==1);
  }

  // Tell all players to close sockets
  for(int i=0;i<num_players;i++){
    int closedFlag=1;
    if(safe_send(socket_array[i],&closedFlag,sizeof(closedFlag),0)){return -1;}
  }


  freeaddrinfo(host_info_list);
  for(int i=0;i<num_players;i++){
    close(socket_array[i]);
  }
  close(socket_fd);

  return 0;
}
