#include "hashtable.h"
#include<iostream>
#include<functional>  // for the hash function.

KV_Store::KV_Store(size_t size) {
    size__ = size;
    table__.resize(size__);
}


void KV_Store::add(std::string &key,std::string &value){

  size_t hash_ind = KV_Store::hash__(key);

  table__[hash_ind].push_back({key,value});

}

std::string KV_Store::get(std::string &key){

  size_t hash_ind = KV_Store::hash__(key);

  for(auto &[k,v] : table__[hash_ind]){
    if(k==key){
      return v;
    }
  }

  return "";

}

void KV_Store::remove(std::string &key){

  size_t hash_ind = KV_Store::hash__(key);

  size_t ind = -1;

  for(size_t i = 0;i<table__[hash_ind].size();i++){
    if(table__[hash_ind][i].first == key){
      ind = i;
      break;
    }
  }

  if(ind==-1)
    return;

  table__[hash_ind].erase(table__[hash_ind].begin()+ind);

  return;

}

size_t KV_Store::hash__(std::string &key)const{

  return std::hash<std::string>{}(key) % size__; 

}
