cmake_minimum_required(VERSION 3.20)
project(GPIO)

set(CMAKE_CXX_STANDARD 20)

add_executable(output output.cpp)
target_link_libraries(output PRIVATE gpiodcxx)
