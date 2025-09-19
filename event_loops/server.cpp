#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<poll.h>

using namespace std;

#define PORT 8080

struct pollfd{  // This is the struct that should be passed to the poll() system call.

  int fd;
  short events;
  short revents;

}

int main(){

  

}



