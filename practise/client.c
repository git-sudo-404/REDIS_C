#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>

int main(){
  
  int fd = socket(AF_INET,SOCK_STREAM,0);

  struct sockaddr_in server_addr = {} ;
  server_addr.sin_family = AF_INET ;
  server_addr.sin_addr.s_addr = htonl(0) ;
  server_addr.sin_port = htons(1234) ;

  
  int rv = connect(fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) ;

  if(rv){
    perror("client : connect()") ;
    exit(1) ;
  }
    
  printf("\nConnected with Server\n");

  chat_with_server(fd) ;
  


  close(fd) ;


}
