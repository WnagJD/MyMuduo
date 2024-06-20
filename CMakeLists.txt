cmake_minimum_required(VERSION 3.0)
project(mymuduo)

#设置编译选项  构建动态库
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fpic")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(. SRC_LIST)

add_library(mymuduo SHARED ${SRC_LIST})
