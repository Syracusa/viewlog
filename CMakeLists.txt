cmake_minimum_required(VERSION 3.16)

project(viewlog)

set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-fsanitize=address -Wno-address-of-packed-member)
add_link_options(-fsanitize=address)

add_executable(viewlog 
                main.c
                viewlog.c
                ctrl.c
                util.c)

add_executable(dummy 
                dummy.c)
