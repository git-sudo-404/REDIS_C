/* Pipelining                                */
/* client (one-by-one)   client (pipeline)   */
/* ────────────────────────────────────────► */
/*  ╲  ↗ ╲  ↗             ╲ ╲↗ ↗             */
/*   ╲╱   ╲╱               ╲╱╲╱              */
/* ────────────────────────────────────────► */
/* server                                    */


#include<stdio.h>
#include<iostream>
#include<vector>
#include<cstdint>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<poll.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define max_msg_len 4096
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
    fd2conn.resize(client_fd+1);
  }

  fd2conn[client_fd] = client_conn;

}

//NOTE: Whenever this 'try_one_read()' function is called the 'conn::incoming' buffer should have 
// atlest one complete request. Or else don't parse anything , just return.
// px --> prefix length, msg --> message /data .
// conn::incoming :
// |px|msg|px|msg|px..
// --> I process this buffer only when it contains atleast on complete |px|msg|.
// --> Make the return type of the 'try_one_read()' boolean , so that it returns true and 
// keeps on executing as long as the prev req was sent. if prev req fails(not enough data in conn::incoming buffer)
// then it doesn't call itself again.

static bool try_one_read(int client_fd){   

  Conn *conn = fd2conn[client_fd];
  if(!conn)return false;

  if(conn->incoming.size()<4)
    return false;   // Needs to read again to get the prefix length.

  int prefix_length;
  memcpy(&prefix_length,conn->incoming.data(),4);

  if(4+prefix_length>conn->incoming.size())
    return false; // Needs to get the full msg.

  memcpy(
        conn->outgoing.data() + conn->outgoing.size(),
        conn->incoming.data()+4,
        prefix_length
        );

  conn->incoming.erase(conn->incoming.begin(),conn->incoming.begin()+4+prefix_length);

  return true;

}

static void handle_read(int client_fd){

  Conn *conn = fd2conn[client_fd];
  if(!conn) return ;
  
  uint32_t bytes_read = read(
                            conn->fd,
                            conn->incoming.data() + conn->incoming.size(),
                            max_msg_len
                            );

  if(bytes_read<0){
    conn->want_close = true;
    return;
  }

  while(try_one_read(client_fd)){}
  
}


static void handle_write(int client_fd){

  Conn *conn = fd2conn[client_fd];

  if(!conn)return;

  uint32_t bytes_written = write(
                                conn->fd,
                                conn->outgoing.data(),
                                conn->outgoing.size()
                                ); 

  if(bytes_written<0){
    conn->want_close = true;
    return;
  }

  return;

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
  std::cout<<"Server listening on PORT : "<<PORT<<std::endl;

  struct pollfd server_pfd = {};
  server_pfd.fd = fd;
  server_pfd.events = POLLIN;
  server_pfd.revents = 0;

  while(1){

    pfds.clear();
    
    pfds.push_back(server_pfd);

    for(int i=3;i<fd2conn.size();i++){
      
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
      
      pfds.push_back(client_pfd);

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
        Conn *conn = fd2conn[pfds[i].fd];
        close(conn->fd);
        pfds[i].fd = -1;
        delete(conn);
        fd2conn[pfds[i].fd] = NULL;
        continue;
      }

    }

  }

  close(fd);

}
