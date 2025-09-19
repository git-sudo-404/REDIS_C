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

// NOTE: No need to redefine it again since it is already imported using the poll.h .
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

  }

}



