# Specify the minimum version of CMake required.
cmake_minimum_required(VERSION 3.10)

# Define the project name and specify the language (CXX for C++).
project(hashtables_1 CXX)

# --- Server Executable ---
# Creates an executable named "ht_1_server".
# It's built from server.cpp AND hashtable.cpp.
add_executable(server server.cpp hashtable.cpp)

# --- Client Executable ---
# Creates a separate executable named "ht_1_client".
# It's built from client.cpp and probably needs the hashtable code too.
add_executable(client client.cpp hashtable.cpp)
