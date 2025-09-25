#include<vector>


class KV_Store{

private :

  std::vector<std::vector<std::string>>mp;

  int size;
  
  static int hash(std::string){}

public : 
  
  static void KV_Store(int size){}

  static void add(std::string key,std::string value){}

  static std::string get(std::string key){}

  static void delete(std::string key){}


};
