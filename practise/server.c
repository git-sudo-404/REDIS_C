#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<stdlib.h>


void process_connection(int client_fd){

  char rbuf[1024] = {};
  char wbuf[1024] = {};

  while(1){
    
    
    read(client_fd,rbuf,sizeof(rbuf));
    printf("\nClient : %s\n",rbuf);

    printf("You : ");
    scanf("%s",wbuf);

    write(client_fd,wbuf,sizeof(wbuf));

    if(strcmp("BYE",wbuf)==0){  
      return ;
    }

  }

}


int main(){

  int fd = socket(AF_INET,SOCK_STREAM,0) ;

  int re_use_address = 1;
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&re_use_address,sizeof(re_use_address)) ;

  struct sockaddr_in server_addr = {} ;

  server_addr.sin_family = AF_INET ;
  server_addr.sin_port = htons(1234) ;
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK) ;

  int rv = bind(fd,(const struct sockaddr*) &server_addr ,sizeof(server_addr)) ;

  if(rv){
    perror("bind()");
    exit(1);
  }

  rv = listen(fd,SOMAXCONN) ;

  if(rv){
    perror("listen()");
    exit(1);
  }
  
  printf("Server listening on PORT : %d\n",ntohs(server_addr.sin_port)) ;

  while(1){
  
    struct sockaddr_in client_addr = {} ;
    socklen_t client_addr_len = sizeof(client_addr) ;
    int client_fd = accept(fd,(struct sockaddr*)&client_addr , &client_addr_len ) ;

    if(client_fd<0){
      perror("client_accpet()");
      exit(1);
    }

    process_connection(client_fd) ;

    close(client_fd);

  }

  close(fd);

}
