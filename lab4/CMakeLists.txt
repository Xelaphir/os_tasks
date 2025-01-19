cmake_minimum_required(VERSION 3.5.0)
project(lab4 VERSION 0.1.0 LANGUAGES C)

add_library(logger SHARED logger.c)
add_library(usb_port SHARED usb_port.c)

add_executable(lab4 test_logger.c)
target_link_libraries(lab4 logger usb_port)