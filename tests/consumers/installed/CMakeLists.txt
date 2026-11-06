cmake_minimum_required(VERSION 3.21)
project(xleak_installed LANGUAGES CXX)

enable_testing()
find_package(xleak CONFIG REQUIRED)

add_executable(consumer ../main.cpp)
target_link_libraries(consumer PRIVATE xleak::xleak)
add_test(NAME consumer COMMAND consumer)
