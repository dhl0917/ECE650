#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <stdio.h>
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
  
  // Connect to server
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
  //send(socket_server, &connectedFlag, sizeof(connectedFlag), 0);
  if(safe_send(socket_server, &connectedFlag, sizeof(connectedFlag), 0)){return -1;}


  int numberMessage[2];
  //recv(socket_server,numberMessage,sizeof(numberMessage),0);
  assert(recv(socket_server,numberMessage,sizeof(numberMessage),MSG_WAITALL)==sizeof(numberMessage));
  int id = numberMessage[0];
  int num_players = numberMessage[1];
  cout<<"Connected as player "<<id<<" out of "<<num_players<<" total players"<<endl;


  // Connect with neighbors
  bool syn = false;
  while(!syn){
    int signal;
    //recv(socket_server,&signal,sizeof(signal),0); // Standby, waiting for orders from the server
    assert(recv(socket_server,&signal,sizeof(signal),MSG_WAITALL)==sizeof(signal));
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
      //recv(socket_left,&connectedFlag,sizeof(connectedFlag),0);
      assert(recv(socket_left,&connectedFlag,sizeof(connectedFlag),MSG_WAITALL)==sizeof(connectedFlag));
      if(connectedFlag==1){
        //send(socket_server,&connectedFlag,sizeof(connectedFlag),0); // indicate the success to the server
        if(safe_send(socket_server,&connectedFlag,sizeof(connectedFlag),0)){return -1;}
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
      socklen_t socket_addr_len = sizeof(socket_addr);
      assert(recv(socket_server,&socket_addr_len,sizeof(socket_addr_len),MSG_WAITALL)==sizeof(socket_addr_len));
      //recv(socket_server,&socket_addr,sizeof(struct sockaddr_storage),0);
      assert(recv(socket_server,&socket_addr,socket_addr_len,MSG_WAITALL)==socket_addr_len);
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
      //send(socket_right, &connectedFlag, sizeof(connectedFlag), 0);
      if(safe_send(socket_right, &connectedFlag, sizeof(connectedFlag), 0)){return -1;}
    }
  }

  // Game start
  bool GAMEOVER = false;
  fd_set rfds;
  int retval;
  potato_t potato;
  memset(&potato, 0, sizeof(potato));
  int max_fdid;
  max_fdid = (socket_left>socket_right)?socket_left:socket_right;
  max_fdid = (socket_server>max_fdid)?socket_server:max_fdid;
  
  srand((unsigned int)time(NULL)+id);
  while(!GAMEOVER){
    FD_ZERO(&rfds);
    FD_SET(socket_server, &rfds);
    FD_SET(socket_left, &rfds);
    FD_SET(socket_right, &rfds);
    

    retval = select(max_fdid+1, &rfds, NULL, NULL, NULL);
    if (retval == -1){
      cerr<<"select()"<<endl;
    }
    if(FD_ISSET(socket_server, &rfds)){
      assert(recv(socket_server,&potato,sizeof(potato),MSG_WAITALL)==sizeof(potato));
    }
    if(FD_ISSET(socket_left, &rfds)){
      assert(recv(socket_left,&potato,sizeof(potato),MSG_WAITALL)==sizeof(potato));
    }
    if(FD_ISSET(socket_right, &rfds)){
      assert(recv(socket_right,&potato,sizeof(potato),MSG_WAITALL)==sizeof(potato));
    }
    if(potato.GAMEOVER){
      GAMEOVER = true;
    }
    else{
      potato.num_hops -= 1;
      potato.index += 1;
      potato.trace[potato.index] = id;

      if(potato.num_hops == 0){
        cout<<"I’m it"<<endl;
        //send(socket_server,&potato,sizeof(potato),0);
        if(safe_send(socket_server,&potato,sizeof(potato),0)){
          return -1;
        }
      }
      else{
        int random_send = rand() % 2; // 0 to the left, 1 to the right
        if(random_send == 0){
          if(id == num_players-1){
            cout<<"Sending potato to "<<0<<endl;
          }
          else{
            cout<<"Sending potato to "<<id+1<<endl;
          }
          //send(socket_left,&potato,sizeof(potato),0);
          if(safe_send(socket_left,&potato,sizeof(potato),0)){
            return -1;
          }
        }
        else{
          if(id == 0){
            cout<<"Sending potato to "<<num_players-1<<endl;
          }
          else{
            cout<<"Sending potato to "<<id-1<<endl;
          }
          //send(socket_right,&potato,sizeof(potato),0);
          if(safe_send(socket_right,&potato,sizeof(potato),0)){
            return -1;
          }
        }
      }
    }
  }
  FD_ZERO(&rfds);

  int gameoverFlag = 1;
  if(safe_send(socket_server,&gameoverFlag,sizeof(gameoverFlag),0)){return -1;}



  int closedFlag=0;
  assert(recv(socket_server,&closedFlag,sizeof(closedFlag),MSG_WAITALL)==sizeof(closedFlag));
  assert(closedFlag==1);
  

  close(socket_left);
  close(socket_right);
  close(socket_listen);
  close(socket_server);
  cout<<"Player "<<id<<" closed"<<endl;
  return 0;
}
