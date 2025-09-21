// REQUEST : 
//
// ┌────┬───┬────┬───┬────┬───┬───┬────┐
// │nstr│len│str1│len│str2│...│len│strn│
// └────┴───┴────┴───┴────┴───┴───┴────┘
//   4B   4B ...   4B ...
//
// nstr --> length of the list of strings.
// len  --> length of the immediate next string.
// str1 --> string.
//
//
// RESPONSE : 
//
// ┌────────┬─────────┐
// │ status │ data... │
// └────────┴─────────┘
//     4B     ...
//
// status --> succes / failure ( if client sends a req of GET age , and age doesn't exist as a key then it's a failure).
// data   --> the requested data if success.




#include<stdio.h>
#include<string.h>
#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>


#define PORT 8080


int main(){

  

}

