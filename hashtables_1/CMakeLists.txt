cmake_minimum_required(VERSION 3.10)
project(hashtables_1 CXX)

# Tell CMake we want to use the C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create a library from your hashtable source files
add_library(kv_store hashtable.cpp)

# Create the server and link it against the library
add_executable(server server.cpp)
target_link_libraries(server PRIVATE kv_store)

# Create the client and link it against the library
add_executable(client client.cpp)
target_link_libraries(client PRIVATE kv_store)
