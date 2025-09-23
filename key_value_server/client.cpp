#include<iostream>
#include<vector>
#include<sys/socket.h>
#include<cstdint>
#include<arpa/inet.h>
#include<string>
#include<unistd.h>

#define PORT 8080
#define BUFFER_SIZE 4096

static void parse_response(int &client_fd){

  // std::cout<<"Inside parse_response()"<<std::endl;

  uint8_t buf[BUFFER_SIZE];
  int ind = 0;

  size_t bytes_read = 0;

  while(ind<4){
    bytes_read = read(client_fd,buf+ind,4);
    ind += bytes_read;
  }

  uint32_t nstr = 0;
  memcpy(&nstr,buf,4);

  // std::cout<<"nstr : "<<nstr<<std::endl;

  nstr--; // status code.
 
  uint32_t status_len = 0;
  ind = 0;
  bytes_read = 0;

  while(ind<4){
    bytes_read = read(client_fd,buf,4);
    ind += bytes_read;
  }

  memcpy(&status_len,buf,4);

  uint32_t status = 0;

  ind = 0;
  bytes_read = 0;

  while(ind<status_len){
    bytes_read = read(client_fd,buf,status_len);
    ind += bytes_read;
  }

  memcpy(&status,buf,status_len);

  std::cout<<"Status : "<<status<<std::endl;
  
  while(nstr--){

    ind = 0;
    bytes_read = 0;

    while(ind<4){
      bytes_read = read(client_fd,buf,4);
      ind += bytes_read;
    }

    size_t len = 0;
    memcpy(&len,buf,4);

    std::string str(len,' ');

    ind = 0;
    bytes_read = 0;
    
    while(ind<len){
      bytes_read = read(client_fd,buf,len);
      ind += bytes_read;
    }

    for(int i=0;i<ind;i++)
      str[i] = buf[i];

    std::cout<<"Recieved Data : "<<str<<std::endl;

  }

  std::cout<<std::endl;

}

static void send_set_req(int &client_fd,std::string &key,std::string &value){

  uint8_t buf[BUFFER_SIZE];
  int ind = 0;
  
  uint32_t nstr = 3;
  memcpy(buf+ind,&nstr,4);
  ind+=4;

  // std::cout<<"nstr : "<<nstr<<std::endl;

  uint32_t keyword_len = 3;
  memcpy(buf+ind,&keyword_len,4);
  ind+=4;

  // std::cout<<"keyword_len : "<<keyword_len<<std::endl;

  std::string keyword = "SET";
  for(int i=0;i<3;i++){
    buf[ind] = keyword[i];
    ind++;
  }

  // std::cout<<"keyword : "<<keyword<<std::endl;

  uint32_t key_len = key.size();
  memcpy(buf+ind,&key_len,4);
  ind+=4;

  // std::cout<<"key_len : "<<key_len<<std::endl;

  for(int i=0;i<key_len;i++){
    buf[ind] = key[i];
    ind++;
  }

  // std::cout<<"key : "<<key<<std::endl;

  uint32_t value_len = value.size();
  memcpy(buf+ind,&value_len,4);
  ind+=4;

  // std::cout<<"value_len : "<<value_len<<std::endl;

  for(int i=0;i<value_len;i++){
    buf[ind] = value[i];
    ind++;
  }

  // std::cout<<"value : "<<value<<std::endl;

  size_t bytes_writen = 0;
  size_t total_length = ind;

  ind = 0;

  while(total_length){

    bytes_writen = write(client_fd,buf+ind,total_length);
    total_length -= bytes_writen;
    ind += bytes_writen;

    // std::cout<<"ind : "<<ind<<" ";

  }

  std::cout<<std::endl;

  // std::cout<<"SET REQUEST SENT SUCCESSFULLY!"<<std::endl;
  
  parse_response(client_fd);

}


static void send_get_req(int &client_fd,std::string &key){

  uint8_t buf[BUFFER_SIZE];
  int ind = 0;
  size_t bytes_writen = 0;

  uint32_t nstr = 2;
  memcpy(buf+ind,&nstr,4);
  ind += 4;

  uint32_t keyword_len = 3;
  memcpy(buf+ind,&keyword_len,4);
  ind+=4;

  std::string keyword = "GET";
  for(int i=0;i<keyword_len;i++){
    buf[ind] = keyword[i];
    ind++;
  }

  uint32_t key_len = key.size();
  memcpy(buf+ind,&key_len,4);
  ind+=4;

  for(int i=0;i<key_len;i++){
    buf[ind] = key[i];
    ind++;
  }

  uint32_t total_length = ind;
  ind = 0;

  while(total_length){
    bytes_writen = write(client_fd,buf+ind,total_length);
    ind += bytes_writen;
    total_length -= bytes_writen;
  }

  parse_response(client_fd);

}

static void send_del_req(int &client_fd,std::string &key){

  uint8_t buf[BUFFER_SIZE];
  int ind = 0;
  uint32_t bytes_writen = 0;

  uint32_t nstr = 2;
  memcpy(buf+ind,&nstr,4);
  ind+=4;

  uint32_t keyword_len = 3;
  memcpy(buf+ind,&keyword_len,4);
  ind+=4;

  std::string keyword = "DEL";
  for(int i=0;i<keyword_len;i++){
    buf[ind] = keyword[i];
    ind++;
  }

  uint32_t key_len = key.size();
  memcpy(buf+ind,&key_len,4);
  ind+=4;

  for(int i=0;i<key_len;i++){
    buf[ind] = key[i];
    ind++;
  }

  uint32_t total_length = ind;
  ind = 0;

  while(total_length){
    bytes_writen = write(client_fd,buf+ind,total_length);
    total_length-=bytes_writen;
    ind+=bytes_writen;
  }

  parse_response(client_fd);

}


int main(){

  int client_fd = socket(AF_INET,SOCK_STREAM,0);

  struct sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  int rv = connect(client_fd,(const struct sockaddr*)&server_addr,sizeof(server_addr));

  if(rv<0){
    perror("Error : connect()");
    close(client_fd);
    exit(1);
  }

  std::cout<<"Client Connected to Server! Client fd : "<<client_fd<<std::endl;
  
  std::string key = "Apple";
  std::string value = "MacBook Pro 16";


  std::cout<<"SET : "<<std::endl;
  send_set_req(client_fd,key,value);
  std::cout<<"GET : "<<std::endl;
  send_get_req(client_fd,key);
  std::cout<<"DEL : "<<std::endl;
  send_del_req(client_fd,key);
  std::cout<<"GET: "<<std::endl;
  send_get_req(client_fd,key);

  close(client_fd);

}
