cmake_minimum_required(VERSION 3.13)
project(PlaneController)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(PlaneController main.cpp)

target_link_libraries(PlaneController ${SDL2_LIBRARIES} "rf24")