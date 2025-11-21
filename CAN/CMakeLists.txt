cmake_minimum_required(VERSION 3.22)
project(CAN)

set(CMAKE_CXX_STANDARD 20)
find_package(kaylordut REQUIRED)

add_executable(send send.cpp can/driver_can.cpp)
target_link_libraries(send PRIVATE ${kaylordut_LIBS})

add_executable(receive receive.cpp can/driver_can.cpp)
target_link_libraries(receive PRIVATE ${kaylordut_LIBS})
