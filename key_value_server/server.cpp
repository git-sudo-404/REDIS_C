// So far...
// ┌─────┬──────┬─────┬──────┬────────
// │ len │ msg1 │ len │ msg2 │ more...
// └─────┴──────┴─────┴──────┴────────
//    4B   ...     4B   ...
//
// Now,
//
// REQUEST : 
//
// ┌────┬───┬────┬───┬────┬───┬───┬────┐
// │nstr│len│str1│len│str2│...│len│strn│
// └────┴───┴────┴───┴────┴───┴───┴────┘
//   4B   4B ...   4B ...
//
//
// nstr --> number of strings / number of items in the list.
// len  --> length of the following string.
// str1 --> the message/data.
//
//  Eg : SET mykey myvalue
//
// RESPONSE : 
//
// ┌────────┬─────────┐
// │ status │ data... │
// └────────┴─────────┘
//     4B     ...



 // Handle requests
 //
 //  3 steps to handle a request:
 //
 //  Parse the command.
 //  Process the command and generate a response.
 //  Append the response to the output buffer.



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
#include<memory>
#include<map>

#define PORT 8080
#define BUFFER_SIZE 4096
#define max_msg_len 4096

// <----------- Store ------------>

std::map<std::string,std::string>store;   // rn use map, implement later.


// <------------ Response ---------->

struct Response{

  uint32_t status;
  std::vector<uint8_t> data;

};


// <------------ QUEUE ----------->

struct Queue{

  uint8_t buffer[BUFFER_SIZE]={};
  size_t front = 0;   // Use size_t for sizes and indices.
  size_t back = 0;
  size_t size = 0;
  
};


static bool add_to_Queue(Queue& q,uint8_t byte){
  
  if(q.size==BUFFER_SIZE)
    return false;
  
  q.buffer[q.back] = byte;
  q.back = (q.back+1)%BUFFER_SIZE;
  q.size++;

  return true;

}

static uint8_t pop_from_Queue(Queue& q){

  if(q.size==0)
    return 0;

  uint8_t rv = q.buffer[q.front];
  q.front = (q.front+1)%BUFFER_SIZE;
  q.size--;
  
  return rv;
}

static uint8_t peek_from_Queue(Queue &q,int ind){

  if(q.size==0 or ind>q.size)
    return 0;

  return q.buffer[(q.front + ind)%BUFFER_SIZE];

}

static bool isFull(Queue &q){
  return q.size == BUFFER_SIZE;
}

static bool isEmpty(Queue &q){
  return q.size == 0;
}


//   <-------------- QUEUE ENDS ------------->


struct Conn{
  
  int fd;

  bool want_read;
  bool want_write;
  bool want_close;


  // struct Queue* incoming = new Queue();    NOTE: This way of defining is fine , but then it allocates the Queue 
  // struct Queue* outgoing = new Queue();          in the heap. So when Conn is deleted, Queue remains. Have to 
  //                                                manually delete them. Instead use a smart pointer from <memory>.

  // std::unique_ptr<Queue> incoming = std::make_unique<Queue>();         <--   Better way.
  // std::unique_ptr<Queue> outgoing = std::make_unique<Queue>();    

  Queue incoming;           // <-- Best way (since the Queue is of fixed size.)
  Queue outgoing;           // The 'incoming' & 'outgoing' lives as long as Conn lives.

};

//      <----------- Global maps for fd->conn and poll args vector ------------>

std::vector<Conn*>fd2conn;
std::vector<pollfd>pfds;

//      <----------- Global maps for fd->conn and poll args vector -- ENDS  ------------>



//      <------------ HANDLE Functions ----------->


static void handle_accept(int server_fd){

  struct sockaddr_in client_addr = {} ;
  socklen_t client_addr_len = sizeof(client_addr);

  int client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_addr_len);
  // Set client socket to non-blocking
  fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL, 0) | O_NONBLOCK);

  std::cout<<"New Client Connestion : "<<client_fd<<std::endl;
  
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

  if(conn->incoming.size<4)
    return false;   // Needs to read again to get the prefix length.

  size_t prefix_length = 0;
  uint8_t pref_bytes[4];
  // memcpy(&prefix_length,conn->incoming.data(),4);
  for(int i=0;i<4;i++){
    prefix_length <<=8;     // 4 bytes of prefix_length ==> uint8_t | uint8_t | uint8_t | uint8_t.
    prefix_length |= peek_from_Queue(conn->incoming,i);
    pref_bytes[i] = peek_from_Queue(conn->incoming,i);
  }

  if(4+prefix_length>conn->incoming.size)
    return false; // Needs to get the full msg.

  // memcpy(
  //       conn->outgoing.data() + conn->outgoing.size(),
  //       conn->incoming.data()+4,
  //       prefix_length
  //       );

  for(int i=0;i<4;i++)    // consume the 4-byte prefix length.
    pop_from_Queue(conn->incoming);

  for(int i=0;i<4;i++)
    add_to_Queue(conn->outgoing,pref_bytes[i]);

  for(int i=0;i<prefix_length;i++){
    add_to_Queue(conn->outgoing,pop_from_Queue(conn->incoming));
  } 

  // conn->incoming.erase(conn->incoming.begin(),conn->incoming.begin()+4+prefix_length);

  return true;

}







static void handle_write(int client_fd){

  Conn *conn = fd2conn[client_fd] ;

  if(!conn)
    return;

  uint8_t buf[BUFFER_SIZE];
  int ind = 0;

  for(int i=0;i<conn->outgoing.size;i++){
    buf[ind] = peek_from_Queue(conn->outgoing,i);
    ind++;
  }

  size_t bytes_written = write(conn->fd,buf,ind);

  if(bytes_written<0){
    if(errno == EAGAIN || errno == EWOULDBLOCK)
      return;
    perror("Error : handle_write()");
    exit(1);
  }

  for(int i=0;i<bytes_written;i++)
    pop_from_Queue(conn->outgoing);

  if(isEmpty(conn->outgoing)){
    conn->want_write = false;
  }

  return;


}

// cmd    ---     Response 
// GET    ---     Status : 200 , data : [val]
// SET    ---     Status : 200 , data : [key,val]
// DEL    ---     Status : 200 , data : []

static void send_response(Conn *conn,struct Response &resp){

  uint8_t buf[BUFFER_SIZE];
  size_t ind = 0;

  if(resp.data.size()>0){   // GET 

    uint32_t nstr = 2; 
    memcpy(buf,&nstr,4);

    uint32_t len = 4;
    memcpy(buf+4,&len,4);

    uint32_t status = resp.status;
    memcpy(buf+8,&status,4);

    len = resp.data.size();
    memcpy(buf+12,&len,4);

    ind = 12 + 4 ;

    for(int i=0;i<len;i++){
      buf[ind] = resp.data[i];
      ind++;
    }

  } else {  // SET, DEl 
   
    // std::cout<<"Inside Send Response : SET/DEL"<<std::endl;

    uint32_t nstr = 1;
    memcpy(buf,&nstr,4);

    uint32_t len = 4;
    memcpy(buf+4,&len,4);

    uint32_t status = resp.status;
    memcpy(buf+8,&status,4);

    ind = 12;

  }
  
  for(int i=0;i<ind;i++)
    add_to_Queue(conn->outgoing,buf[i]);

  // std::cout<<"Sent Response to Client"<<std::endl;

  return;

}

static uint32_t parse_set(Conn *conn,size_t &keyword_len){

  // std::cout<<"Inside Parse Set"<<std::endl;

  if(conn->incoming.size < 4+4+keyword_len+4)
    return -1; // needs read.

  size_t key_len;
  uint8_t temp_buf[4];
  for(int i=0;i<4;i++){
    // key_len<<=8;
    temp_buf[i] = peek_from_Queue(conn->incoming,4+4+keyword_len+i);
  }

  memcpy(&key_len,temp_buf,4);

  // std::cout<<"key_len : "<<key_len<<std::endl;

  if(conn->incoming.size< 4 + 4 + keyword_len + 4 + key_len)
    return -1; // needs read.

  std::string key(key_len,' ');
  for(int i=0;i<key_len;i++){
    key[i] = peek_from_Queue(conn->incoming,4+4+keyword_len+4+i);
  }

  // std::cout<<"key : "<<key<<std::endl;

  if(conn->incoming.size < 4 + 4 + keyword_len + 4 + key_len + 4)
    return -1; // needs read.

  uint32_t value_len;
  for(int i=0;i<4;i++){
    // value_len<<=8;
    temp_buf[i] = peek_from_Queue(conn->incoming,4 + 4 + keyword_len + 4 + key_len + i);
  }

  memcpy(&value_len,temp_buf,4);

  // std::cout<<"value_len : "<<value_len<<std::endl;

  if(conn->incoming.size < 4 + 4 + keyword_len + 4 + key_len + 4 + value_len)
    return -1; // needs read.

  std::string value(value_len,' ');
  for(int i=0;i<value_len;i++){
    value[i] = peek_from_Queue(conn->incoming,4+4+keyword_len+4+key_len+4+i);
  }

  store[key] = value;

  uint32_t buf_consume = 4 + 4 + keyword_len + 4 + key_len + 4 + value_len ;

  struct Response response_set = {};
  response_set.status = 200;

  send_response(conn,response_set);

  // std::cout<<"SET KEY : "<<key<<" VAL : "<<store[key]<<std::endl;

  return buf_consume ;

}


static uint32_t parse_get(Conn *conn,size_t &keyword_len){

  if(conn->incoming.size<4+4+keyword_len+4)
    return -1 ; // needs read.

  uint32_t key_len;
  uint8_t temp_buf[4];
  for(int i=0;i<4;i++){
    // key_len<<=8;
    temp_buf[i]=peek_from_Queue(conn->incoming,4+4+keyword_len+i);
  }

  memcpy(&key_len,temp_buf,4);

  if(conn->incoming.size<4+4+keyword_len+4+key_len)
    return -1;  // needs read.

  std::string key(key_len,' ');
  for(int i=0;i<key_len;i++){
    key[i] = peek_from_Queue(conn->incoming,4+4+keyword_len+4+i);
  }

  uint32_t buf_consume = 4 + 4 + keyword_len + 4 + key_len ;

  struct Response response_get = {};
  response_get.status = 200;
 
  if(store.find(key)==store.end()){
    
    response_get.status = 404;

    std::string data = "Key Not Found!";

    size_t len = data.size();

    for(int i=0;i<len;i++)
      response_get.data.push_back(data[i]);

    send_response(conn,response_get);

    return buf_consume;
  }

  std::string value = store[key] ;

  for(int i=0;i<value.size();i++)
    response_get.data.push_back(value[i]);

  send_response(conn,response_get);

  return buf_consume;

}

static uint32_t parse_del(Conn *conn,size_t &keyword_len){
  
  if(conn->incoming.size<4+4+keyword_len+4)
    return -1; // needs read 

  uint32_t key_len;
  uint8_t temp_buf[4];
  for(int i=0;i<4;i++){
    // key_len<<=8;
    temp_buf[i] = peek_from_Queue(conn->incoming,4+4+keyword_len+i);
  }

  memcpy(&key_len,temp_buf,4);

  if(conn->incoming.size < 4 + 4 + keyword_len + 4 + key_len)
    return -1;  // needs read.

  std::string key(key_len,' ');

  for(int i=0;i<key_len;i++){
    key[i] = peek_from_Queue(conn->incoming,4+4+keyword_len+4+i);
  }

  store.erase(key);

  uint32_t buf_consume = 4 + 4 + keyword_len + 4 + key_len ;

  struct Response response_del = {};
  response_del.status = 200;
  
  send_response(conn,response_del);

  return buf_consume;

}


static uint32_t parse_req(Conn *conn){

  // std::cout<<"Inside Parse Req"<<std::endl;

  if(conn->incoming.size<4)
    return -1;

  uint32_t nstr = 0;
  uint8_t temp_buf[4];
  for(int i=0;i<4;i++){
    
    temp_buf[i] = peek_from_Queue(conn->incoming,i);

  }

  memcpy(&nstr,temp_buf,4);

  // std::cout<<"nstr : "<<nstr<<std::endl;


  if(conn->incoming.size<4+4)
    return -1; // needs read.

  size_t keyword_len = 0;
  for(int i=4;i<8;i++){
    // keyword_len<<=8;
    temp_buf[i-4]=peek_from_Queue(conn->incoming,i);
  }

  memcpy(&keyword_len,temp_buf,4);

  if(conn->incoming.size<4+4+keyword_len)
    return -1; // needs read. 

  std::string keyword(keyword_len,' '); // GET/SET/DEL.
  for(int i=0;i<keyword_len;i++){
    keyword[i] = peek_from_Queue(conn->incoming,4+4+i);
  }

  if(keyword=="SET"){
    return parse_set(conn,keyword_len);
  } else if(keyword=="GET"){
    return parse_get(conn,keyword_len);
  } else if(keyword=="DEL"){
    return parse_del(conn,keyword_len);
  }
 
  return -1;

}


static void handle_read(int client_fd){

  // std::cout<<"Inside handle_read()"<<std::endl;
  
  Conn *conn = fd2conn[client_fd];
  if(!conn )
    return; // read again.
 
  uint8_t buf[BUFFER_SIZE];

  size_t bytes_read = read(conn->fd,buf,BUFFER_SIZE) ;

  if(bytes_read<=0){
    conn->want_close = true;
    return;
  }

  for(int i=0;i<bytes_read;i++)
    add_to_Queue(conn->incoming,buf[i]);

  uint32_t rv = parse_req(conn);

  if(rv<0){
    return; //needs read.
  } else {
    for(int i=0;i<rv;i++)
      pop_from_Queue(conn->incoming);
  }

  if(conn->outgoing.size)
    conn->want_write = true;

}




//      <------------ HANDLE Functions -- ENDS  ----------->




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
