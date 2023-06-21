cmake_minimum_required(VERSION 3.16)

project(viewlog)

set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-fsanitize=address -Wno-address-of-packed-member)
add_link_options(-fsanitize=address)

add_executable(viewlog 
                viewlog.c)

add_executable(dummy 
                dummy.c)
