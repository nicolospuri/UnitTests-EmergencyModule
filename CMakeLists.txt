cmake_minimum_required(VERSION 4.0)
project(Task0 C)

set(CMAKE_C_STANDARD 11)

add_executable(Task0
        src/emergency_module.c
        src/emergency_module.h
        test/test_emergency_module.c)

enable_testing()

add_test(NAME test_emergency_module COMMAND test)