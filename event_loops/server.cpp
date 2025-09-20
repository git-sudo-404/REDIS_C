#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<poll.h>
#include<vector>
#include<cstdint>
#include<iostream>
#include<fcntl.h>

// using namespace std;

#define PORT 8080

//NOTE: We are using prefix-length network packets here, meaning a data sent over a network has the length of it 
//attatched to it as a prefix.

// We are creating an echo server here.

static const size_t k_max_msg = 4096; // 4 KB

// No need to redefine it again since it is already imported using the poll.h .
// struct pollfd{  // This is the struct that should be passed to the poll() system call.
//
//   int fd;
//   short events;
//   short revents;
//
// }

struct Conn{ // Needed to maintain the state of each connection between each iteration of the event loop.
 
  int fd = -1 ;
  
  bool want_read = false ;
  bool want_write = false ;
  bool want_close = false ;

  std::vector<uint8_t> incoming ; // Data read but not yet processed.
  std::vector<uint8_t> outgoing ; // Date to be written out.
    
};

std::vector<Conn*>fd2conn(3,NULL); // maps the fd to the respective conn struct.
// first 2 are NULL , since thd val of fd starts from 3.
// 1 and 2 are assigned for the terminal inputs and outputs.

std::vector<pollfd>pfds;

static void fd_set_nb(int fd) {     // used to make the socket non - bllocking.
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}



static bool echo(Conn *conn){

  if(conn->incoming.size()<4){   // needs to read again to even get the prefix length.
    return false;   
  }

  int len;
  memcpy(&len,conn->incoming.data(),4); // copy the length.

  if(conn->incoming.size()<len+4){  // need to read again to get the full data.
    return false;
  }

  conn->outgoing.insert(
                        conn->outgoing.end(),
                        conn->incoming.data(),
                        conn->incoming.data()+4+len 
                        ) ;
  
  conn->want_write = true;
  conn->want_read = false;

  conn->incoming.erase(conn->incoming.begin(),conn->incoming.begin()+4+len);

  return true;

}





// incoming buffer --> whatever data i get , i immediatly store in it.

static void handle_read(Conn *conn){

  static uint8_t buf[64*1024];   // uint8_t -- represents a byte (8-bits).

  int bytes_read = read(conn->fd,buf,sizeof(buf)) ;

  if(bytes_read<=0){
    conn->want_close = true;  //Error occured.
    return;
  }
 
  conn->incoming.insert(conn->incoming.end(),buf,buf+bytes_read) ;

  echo(conn); 

}

static void handle_write(Conn *conn){

  assert(conn->outgoing.size()>0);

  size_t bytes_written;

  bytes_written = write(conn->fd,conn->outgoing.data(),conn->outgoing.size());

  if(bytes_written<0){
    conn->want_close = true;
    return;
  }

  conn->outgoing.erase(conn->outgoing.begin(),conn->outgoing.begin()+bytes_written);

  if(conn->outgoing.size()==0){
    conn->want_write = false;
    conn->want_read = true;
    return;
  }

}


int main(){

  int fd = socket(AF_INET,SOCK_STREAM,0);
  if(fd<0){
    perror("Error : socket()");
    exit(1);
  }

  int setsockoptval = 1;
  setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&setsockoptval,sizeof(setsockoptval));

  struct sockaddr_in server_addr = {};

  server_addr.sin_family = AF_INET ;
  server_addr.sin_port = htons(PORT) ;
  server_addr.sin_addr.s_addr = INADDR_ANY ;

  int rv = bind(fd,(const struct sockaddr*)&server_addr,sizeof(server_addr));
  if(rv<0){
    perror("Error : bind()");
    exit(1);
  }

  fd_set_nb(fd);  // made the socket non-blocking on accept().

  rv = listen(fd,SOMAXCONN) ;
  if(rv<0){
    perror("Error : listen()");
    exit(1);
  }

  std::cout<<"Server listening on PORT : "<<PORT<<std::endl ;

  struct pollfd listening_pfd = {} ;
  listening_pfd.fd = fd ;
  listening_pfd.events = POLLIN ;
  listening_pfd.revents = 0 ;


  while(1){
   
    pfds.clear();

    pfds.push_back(listening_pfd) ;

    // Before calling poll() , add the clients from the fd2conn to listen for their read and write needs also

    for(Conn *conn : fd2conn){

      if(!conn)
        continue;

      struct pollfd pfd = {};
      pfd.fd = conn->fd;
      pfd.events = (conn->want_read ? POLLIN : 0) | (conn->want_write ? POLLOUT : 0) ;
      pfd.revents |= POLLERR ;

      pfds.push_back(pfd);

    }

    int number_of_events = poll(pfds.data(),pfds.size(),-1) ; 

    // NOTE: Timout - 0 : listen on all the fds and return immediatly .
    // Having a Timout of 0 does work, coz if there's no event it returns immediatly and 
    // Then once again poll() is called coz of the while loop.
    // This causes the CPU to be executing always even when there's nothing really happening.
    // So , instead it's better to use -1 , which keeps on listening untill there's atleast one 
    // event that triggers it happens. Here , a new client connection being made.

    if(number_of_events<0){
      perror("Error : poll()");
      break;
    }

    // Check the listening socket for new client connection.
    if(pfds[0].revents & POLLIN){
      struct sockaddr_in client_addr = {};
      socklen_t client_addr_len = sizeof(client_addr) ;
      
      int client_fd = accept(fd,(struct sockaddr*)&client_addr,&client_addr_len) ;
      if(client_fd<0){
        perror("Error : accept()");
      } else {
        std::cout<<"New Connection made with Client : "<<client_fd<<std::endl;
        
        fd_set_nb(client_fd) ;

        struct pollfd client_pfd = {};
        client_pfd.fd = client_fd ;
        client_pfd.events = POLLIN ; // Monitor for incoming data .
        client_pfd.revents = 0 ;

        pfds.push_back(client_pfd) ;
        
        Conn *client_conn = new Conn();  // NOTE: Allocate new conn on the heap use new keyword. Don't use 'struct Conn client_conn'.
                                 //  That would allocate it on the stack.
        client_conn->fd = client_fd;
        client_conn->want_read = true;
        client_conn->want_write = false;
        client_conn->want_close = false;

        if(fd2conn.size()<=(size_t)client_fd){
          fd2conn.resize(client_fd+1);
        }

        fd2conn[client_fd] = client_conn;

      }
      
    }

    // Loop through all the client fds and check for data.
    for(int i=1;i<pfds.size();i++){

      uint32_t ready = pfds[i].revents ;
      Conn* conn = fd2conn[pfds[i].fd] ;

      if(!conn)continue;

      if(ready & POLLIN){
        handle_read(conn) ;
      } 

      if(ready & POLLOUT){
        handle_write(conn) ;
      }

      if((ready & POLLERR) || conn->want_close){
        (void)close(conn->fd);
        fd2conn[conn->fd] = NULL;
        delete conn;
      }

    } 
         


  }

  close(fd) ;

}



