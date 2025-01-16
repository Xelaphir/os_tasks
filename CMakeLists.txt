cmake_minimum_required(VERSION 3.5.0)
project(OsTasks VERSION 0.1.0 LANGUAGES C)

add_library(background_prog SHARED background_prog.c)

add_executable(OsTasks main.c)
target_link_libraries(OsTasks background_prog)