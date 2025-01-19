cmake_minimum_required(VERSION 3.5.0)
project(lab3 VERSION 0.1.0 LANGUAGES C)

add_library(counter SHARED counter.c)

add_executable(lab3 test_counter.c)
target_link_libraries(lab3 counter)