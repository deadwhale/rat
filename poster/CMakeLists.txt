cmake_minimum_required(VERSION 3.0.0)
project(poster)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(lib2ch STATIC src/2ch.cpp src/utils.cpp src/menu.cpp)
add_library(libs STATIC src/2ch.cpp src/utils.cpp src/menu.cpp)
add_executable(poster main.cpp)

target_link_libraries(poster PRIVATE lib2ch jsoncpp cpr curl pthread)