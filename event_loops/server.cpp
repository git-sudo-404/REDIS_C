#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<poll.h>

using namespace std;

#define PORT 8080

// No need to redefine it again since it is already imported using the poll.h .
// struct pollfd{  // This is the struct that should be passed to the poll() system call.
//
//   int fd;
//   short events;
//   short revents;
//
// }

vector<pollfd>pfds;

int main(){

  int fd = socket(AF_INET,SOCK_STREAM,0);
  if(fd<0){
    perror("Error : socket()");
    exit(1);
  }

  int setsockoptval = 1;
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&setsockoptval,sizeof(setsockoptval));

  struct sockaddr_in server_addr = {};

  server_addr.sin_family = AF_INET ;
  server_addr.sin_port = htons(PORT) ;
  server_addr.sin_addr.s_addr = INADDR_ANY ;

  int rv = bind(fd,(const struct sockaddr*)&server_addr,sizeof(server_addr));
  if(rv<0){
    perror("Error : bind()");
    exit(1);
  }

  rv = listen(fd,SOMAXCONN) ;
  if(rv<0){
    perror("Error : listen()");
    exit(1);
  }

  cout<<"Server listening on PORT : "<<PORT<<endl ;

  srtruct pollfd listening_pfd = {} ;
  listening_pfd.fd = fd ;
  listening_pfd.events = POLLIN ;
  listening_pfd.revents = 0 ;

  pfds.pb(listening_pfd) ;

  while(1){
    
    int number_of_events = poll(pfds,pfds.size(),-1) ; 

    // NOTE: Timout - 0 : listen on all the fds and return immediatly .
    // Having a Timout of 0 does work, coz if there's no event it returns immediatly and 
    // Then once again poll() is called coz of the while loop.
    // This causes the CPU to be executing always even when there's nothing really happening.
    //
    // So , instead it's better to use -1 , which keeps on listening untill there's atleast one 
    // event that triggers it happens. Here , a new client connection being made.

    if(number_of_events<0){
      perror("Error : poll()");
      break;
    }

    // Check the listening socket for new client connection.
    if(pfds[0].revents & POLLIN){
      struct sockaddr_in client_addr = {};
      socklen_t client_addr_len = sizeof(client_addr) ;
      
      int client_fd = accept(fd,(struct sockaddr*)&client_addr,&client_addr_len) ;
      if(client_fd<0){
        perror("Error : accept()");
      } else {
        cout<<"New Connection made with Client : "<<client_fd<<endl;
        
        struct pollfd client_pfd = {};
        client_pfd.fd = client_fd ;
        client_pfd.events = POLLIN ; // Monitor for incoming data .
        client_pfd.revents = 0 ;

        pfds.pb(client_pfd) ;
      }

    }

    // Loop through all the client fds and check for data.
    // new struct is needed for mainting the incoming and outgoing data of each client. 

  }

}



