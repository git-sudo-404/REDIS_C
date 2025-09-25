#include "hashtable.h"
#include<iostream>

KV_Store::KV_Store(size_t size){
  size__ = size ;
  table__.resize(size__);
}
