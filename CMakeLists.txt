cmake_minimum_required(VERSION 3.1)

project(SuperPool)

set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14" )
include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(./src SRC_LIST)

add_library(SuperPool SHARED ${SRC_LIST})

