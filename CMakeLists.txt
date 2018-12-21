cmake_minimum_required(VERSION 3.12)
project(Bank)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpqxx -lpq")

find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

add_executable(Bank main.cpp Client.cpp Client.h Db.cpp Db.h Account.cpp Account.h)