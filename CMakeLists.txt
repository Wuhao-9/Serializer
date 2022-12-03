cmake_minimum_required(VERSION 3.0)

project(Yazi-serializer)

set(CMAKE_CXX_FLAGS "-std=c++11")

include_directories(Serializer)

aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC)

add_executable(${PROJECT_NAME} ${SRC})