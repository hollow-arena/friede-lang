cmake_minimum_required(VERSION 3.20)
project(friede LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(interpreter src/tokenizer.c
    src/parsemitter.cpp
    src/main.c
    src/vm.h)

target_compile_definitions(interpreter PRIVATE $<$<CONFIG:Debug>:DEBUG>)
