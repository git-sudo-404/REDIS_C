#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>

int read_full(int fd, char* rbuf ,size_t n){
  
  while(n>0){
    
    ssize_t rv = read(fd,rbuf,n);

    if(rv<=0)
      return 0;

    rbuf += rv;
    n -= (size_t)rv;

  }
  
  return 1;

}

int write_full(int fd, char* wbuf ,size_t n){

  while(n>0){
    
    ssize_t rv = write(fd,wbuf,n);
    
    if(rv<=0)
      return 0;

    wbuf += rv;
    n -= (size_t)rv;

  }

  return 1;

}

void chat_with_client(int client_fd){

  int msg_len = 0;

  int rv = read_full(client_fd,(char *)&msg_len,4);
  if(!rv){
    perror("chat_with_client()");
    exit(1);
  }

  if(msg_len>4096){
    perror("Message from Client is too long");
    exit(1);
  }

  char rbuf[4096];
  rv = read_full(client_fd,rbuf,msg_len);

  rbuf[msg_len] = '\0';

  printf("\n\tMessage from CLient : %s\n",rbuf);
  
  char wbuf[4096];

  printf("\n\tMessage to Client : ");
  scanf("%s",&wbuf[4]);
  
  int len = strlen(&wbuf[4]);
  
  memcpy(wbuf,&len,4);

  write_full(client_fd,wbuf,len+4) ;
  


} 

void chat_with_server(int fd){
  
  
  char wbuf[4096] ;

  printf("\n\tMessage to Server : ") ;
  scanf("%4091s",&wbuf[4]) ;

  int msg_len = strlen(&wbuf[4]) ;

  memcpy(wbuf,&msg_len,4);

  write_full(fd,wbuf,msg_len+4) ;

  char rbuf[4096];

  msg_len = 0;

  int rv = read_full(fd,(char *)&msg_len,4);

  rv = read_full(fd,&rbuf[4],msg_len) ;

  

  printf("\n\tMessage from Server : %s\n",&rbuf[4]);
      

}


void one_response(int client_fd){

  char rbuf[4096];

  int len = 0;

  read_full(client_fd,(char*)&len,4);

  read_full(client_fd,rbuf,len);

  printf("\n\tMessage from client : %s",rbuf) ;

  char wbuf[4096];

  memcpy(wbuf,&len,4);

  memcpy(&wbuf[4],rbuf,len);

  printf("\nMessage sent back : %s\n",&wbuf[4]);

  write_full(client_fd,wbuf,len+4);

}
