#include<sys/socket.h>  // socket 
#include<stdio.h>       // read , write 
#include<unistd.h>      // close 
#include<string.h>      // strlen 
#include<netinet/in.h>  // struct sockaddr_in , htons ,... 

static void do_something(int connfd){

  char buffer[1024] = {};

  ssize_t n = read(connfd,buffer,sizeof(buffer) - 1);
  
  if(n<0){
    perror("error : read()");
    return;
  }

  printf("\n\t Simon says : %s\n",buffer) ; 

  char wbuf[] = "King of the World" ; 

  write(connfd,wbuf,strlen(wbuf));


/* ssize_t read(int fd, void *buf, size_t len); */
/* ssize_t recv(int fd, void *buf, size_t len, int flags);         // read */
/* ssize_t write(int fd, const void *buf, size_t len); */
/* ssize_t send(int fd, const void *buf, size_t len, int flags);   // write */
// send and recv has an extra optional flags 

}



int main(){
  
  // Step 1 : Create a file descriptor for a new socket
  int fd = socket(AF_INET,SOCK_STREAM,0);

  // Step 2 : Configure the socket use setsockopt to reuse the same IP and PORT , 
  //              --> When the server restarts (Connection is closed and opened again)
  //                  the TCP Connection would not have been closed immediatly , it would be in TIME_WAIT state , (see SO_REUSEADDR)
  //                  ans it'll throw some error saying the PORT is already in use, by using SO_REUSEADDR we tell the OS , it's ok
  //                  and reuse the same address 

  int reuse_address_val = 1; // 1  - indicates the SO_REUSEADDR is ON 
                            // SO_REUSEADDR accepts 1 or 0 

  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&reuse_address_val,sizeof(reuse_address_val));

  // Step 3 : Bind the socket to an Address 
  //            --> struct sockadrr_in is specially designed struct to hold all the neccessary info for an IPv4 network address 

  struct sockaddr_in bind_address = {};   // {} -- initializes all fields to 0 

  //  struct sockaddr_in {
  //      uint16_t       sin_family; // AF_INET
  //      uint16_t       sin_port;   // port in big-endian
  //      struct in_addr sin_addr;   // IPv4
  //  };
  //  struct in_addr {
  //      uint32_t       s_addr;     // IPv4 in big-endian
  //  };


   
  // Setting the address 

  bind_address.sin_family = AF_INET ; // AF_INET - IPv4 , AF_INET6 - IPv6 
  bind_address.sin_port = htons(1234) ; // bind the PORT number, htons - host to network byte order short 
  bind_address.sin_addr.s_addr = htonl(0) ; // 0 -- wildcard address for 0.0.0.0   
  //                                            --> set to wildcard address (0.0.0.0) , which enables it to listen to all the IP's 
  //                                              (i.e) a client can connect to the server from any IP 

  // Perform the binding 

  int rv = bind(fd,(const struct sockaddr*)&bind_address,sizeof(bind_address)) ;  // 0 - on success , non - zero value - on failure 
  
  if(rv){perror("bind()");}

  // Step 4 : Listen -- here is where the actual socket is created 

  rv = listen(fd,SOMAXCONN); // the 2nd argument is the size of the queue  -- SOMAXCONN is 4096 in Linux 

  if(rv){perror("listen()");}

  printf("\nServer Listeing on PORT : %d\n",ntohs(bind_address.sin_port));

  while(1){
  
    struct sockaddr_in client_addr = {} ;
    socklen_t addrlen = sizeof(client_addr) ; 
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen) ;      // connfd has the file descriptor of the client connection 
    

    if(connfd<0){
      perror("accept()");
    }

    do_something(connfd);

    close(connfd) ;


  } 
  

}
