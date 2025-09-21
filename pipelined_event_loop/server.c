#include<stdio.h>
#include<iostream>
#include<vector>
#include<cstdint>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<fcnt.h>
#include<poll.h>

#define PORT 8080
#define BUFFER_SIZE 4096

struct Conn{
  
  int fd;

  bool want_read;
  bool want_write;
  bool want_close;

  std::vector<uint8_t>incoming,outgoing;

};

std::vector<Conn*>fd2conn;
std::vector<pollfd>pfds;

static void handle_accept(int server_fd){

  struct sockaddr_in client_addr = {} ;
  socklen_t client_addr_len = sizeof(client_addr);

  int client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_addr_len);
  // Set client socket to non-blocking
  fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK);
  
  Conn *client_conn = new Conn(); // Allocates on the heap.

  client_conn->fd = client_fd;
  client_conn->want_read = true;
  client_conn->want_write = false;
  client_conn->want_close = false;

  if(fd2conn.size()<=client_fd){
    fd2conn.resize(client_fd);
  }

  fd2conn[client_fd] = client_conn;

}

static void handle_read(int client_fd){

  Conn *conn = fd2conn[client_fd];
  if(!conn)
    return ;



}


int main(){

  int fd = socket(AF_INET,SOCK_STREAM,0) ;
  
  struct sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET ;
  server_addr.sin_port = htons(PORT) ;
  server_addr.sin_addr.s_addr = INADDR_ANY ;

  int setsockopt_val = 1;
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&setsockopt_val,sizeof(setsockopt_val));

  int rv = bind(fd,(const struct sockaddr*)&server_addr,sizeof(server_addr)) ;

  listen(fd,SOMAXCONN);
  // Set listening socket to non-blocking
  fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0)|O_NONBLOCK);
  cout<<"Server listening on PORT : "<<PORT<<endl;

  struct pollfd server_pfd = {};
  server_pfd.fd = fd;
  server_pfd.events = POLLIN;
  server_pfd.revents = 0;

  while(1){

    pfds.clear();
    
    pfds.push_back(server_pfd);

    for(int i=3;i<=fd2conn.size();i++){
      
      Conn *conn = fd2conn[i];
      
      if(!conn)
        continue;
      
      struct pollfd client_pfd = {};
    
      client_pfd.fd = conn->fd;
      
      if(conn->want_read)
        client_pfd.events |= POLLIN;

      if(conn->want_write)
        client_pfd.events |= POLLOUT;

      if(conn->want_close){
        close(conn->fd);
        fd2conn[i] = NULL;
        delete(conn);
        continue;
      }
      
      pfds.pb(client_pfd);

    }

    int num_events = poll(pfds.data(),pfds.size(),-1);

    if(pfds[0].revents & POLLIN){

      handle_accept(pfds[0].fd);

    }

    for(int i=1;i<pfds.size();i++){
      
      if(pfds[i].revents & POLLIN)
        handle_read(pfds[i].fd);

      if(pfds[i].revents & POLLOUT)
        handle_write(pfds[i].fd);

      if(pfds[i].revents & POLLERR){
        fd2conn[pfds[i].fd] = NULL;
        continue;
      }

    }

  }

  close(fd);

}
