cmake_minimum_required(VERSION 3.5.0)
project(lab2 VERSION 0.1.0 LANGUAGES C)

add_library(bg_proc SHARED bg_proc.c)

add_executable(lab2 test_bg_proc.c)
target_link_libraries(lab2 bg_proc)