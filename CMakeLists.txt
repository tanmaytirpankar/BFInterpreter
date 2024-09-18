cmake_minimum_required(VERSION 3.22)
project(BFInterpreter)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

add_executable(brainfuck_interpreter_cpp brainfuck_interpreter.cpp)
add_executable(brainfuck_interpreter_c brainfuck_interpreter.c)
add_executable(brainfuck_interpreter_c_threaded brainfuck_interpreter_c_threaded.c)