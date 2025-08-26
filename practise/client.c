#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<stdlib.h>



int main(){

  int fd = socket(AF_INET,SOCK_STREAM,0) ;

  struct sockaddr_in server_addr = {} ;

  server_addr.sin_family = AF_INET ;
  server_addr.sin_port = htons(1234) ;
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK) ;


  int rv = connect(fd,(const struct sockaddr*) &server_addr,sizeof(server_addr)) ;

  if(rv){
    perror("Client : connect()") ;
    exit(1) ;
  }

  char rbuf[1024] = {} ;
  char wbuf[1024] = {} ;


  while(1){
    
    printf("You : ");
    scanf("%s",wbuf);

    write(fd,wbuf,sizeof(wbuf));

    read(fd,rbuf,sizeof(rbuf));
    printf("\nServer : %s\n",rbuf);
  
  } 
  
  printf("Connection Closing...");
  close(fd) ;
    

}
