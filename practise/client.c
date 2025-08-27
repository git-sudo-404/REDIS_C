#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include "utils.h"


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
  printf("\n\tMessage to Server : ");
  
  char wbuf[4096];

  scanf("%4091s",&wbuf[4]);

  int len = strlen(&wbuf[4]);

  memcpy(wbuf,&len,4);

  wbuf[len+5] = '\0';
  
  write_full(fd,wbuf,len+5);

  char rbuf[4096];

  rv = read_full(fd,rbuf,4);

  memcpy(&len,rbuf,4);

  rv = read_full(fd,rbuf,len);

  printf("\n\tMessage from Server : %s",rbuf);

  close(fd) ;


}
