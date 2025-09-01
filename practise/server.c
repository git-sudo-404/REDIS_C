#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include "utils.h"


int main(){

  int fd = socket(AF_INET,SOCK_STREAM,0);
  printf("The server fd is : %d",fd);

  int re_use_addr = 1;
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&re_use_addr,sizeof(re_use_addr));

  struct sockaddr_in server_addr = {};

  server_addr.sin_family = AF_INET ;
  server_addr.sin_port = htons(1234) ;
  server_addr.sin_addr.s_addr = htonl(0) ;

  int rv = bind(fd,(const struct sockaddr*) &server_addr,sizeof(server_addr)) ;

  if(rv){
    perror("bind()");
    exit(1);
  }

  rv = listen(fd,SOMAXCONN) ;

  if(rv<0){
    perror("listen()");
    exit(1);
  }

  printf("\nServer Running on PORT : %d\n",server_addr.sin_port) ;
  

  while(1){
  
    struct sockaddr_in client_addr = {} ;

    socklen_t client_addr_len = sizeof(client_addr) ;

    int client_fd = accept(fd,(struct sockaddr*) &client_addr ,&client_addr_len) ;
  
		one_response(client_fd) ;

    close(client_fd) ;

  }

  close(fd) ;

}
