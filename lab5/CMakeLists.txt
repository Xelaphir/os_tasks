cmake_minimum_required(VERSION 3.5.0)
project(lab5 VERSION 0.1.0 LANGUAGES C)

add_library(logger SHARED logger.c)
add_library(usb_port SHARED usb_port.c)
add_library(database SHARED database.c)
add_library(http_server SHARED http_server.c)


add_executable(lab5 test_http_server.c)
target_link_libraries(lab5 logger usb_port database http_server)