#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>


int main(){

  int fd = socket(AF_INET,SOCK_STREAM,0);

  struct sockaddr_in server_addr = {};

  server_addr.sin_family = AF_INET ;
  server_addr.sin_port = htons(1234) ;
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  int rv = connect(fd,(const struct sockaddr *)&server_addr,sizeof(server_addr)) ; 

  if(rv){perror("client : connect()");}

  char msg[] = "Hi There from Client" ;

  write(fd,msg,strlen(msg));

  char rbuf[64]={};

  ssize_t n = read(fd,rbuf,sizeof(rbuf) - 1);

  if(n<0){
    perror("client : read()");
  }

  printf("\nMessage from Server : %s\n",rbuf);
  
  close(fd);

} 
