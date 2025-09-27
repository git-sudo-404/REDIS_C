#pragma once


#include<vector>
#include<string>
#include<utility>

class KV_Store{

private :

  std::vector<std::vector<std::pair<std::string,std::string>>>table__;

  size_t size__;
  
  size_t hash__(std::string &key)const;

public : 
  
  explicit KV_Store(size_t size = 1024);  // Constructor must not be static and must not return any type not even void.

  void add(std::string &key,std::string &value);

  std::string get(std::string &key);

  void remove(std::string &key);


};
