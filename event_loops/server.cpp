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

struct pollfd{  // This is the struct that should be passed to the poll() system call.

  int fd;
  short events;
  short revents;

}

vector<pollfd>fds;

int main(){

  int fd = socket(AF_INET,SOCK_STREAM,0);

  int setsockoptval = 1;
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDRESS,&setsockoptval,sizeof(setsockoptval));

  struct sockaddr_in server_addr = {};

  server_addr.sin_family = AF_INET ;
  server_addr.sin_port = htons(PORT) ;
  server_addr.sin_addr.s_addr = INADDR_ANY ;

  int rv = bind(fd,(const struct sockaddr*)&server_addr,sizeof(server_addr));

  listen(fd,SOMAXCONN) ;

  while(1){

    fds.clear();

    struct listening_fd = {} ;
    listening_fd.fd = fd ;
    listening_fd.events = POLLERR | POLLIN ;
    listening_fd.revents = POLLERR ;

    fds.pb(listening_fd); // add the listening fd .

    struct sockaddr_in client_addr = {} ;
    socklen_t client_addr_len;

    int client_fd = accept(fd,(struct sockaddr*)&client_addr,&client_addr_len);

    
  
    nfds_t nfds = fds.size() ;

    int reqs = poll(fds.begin(),nfds,0) ; // timeout 0 : non - blocking ;

  

  }

}



