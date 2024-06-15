cmake_minimum_required(VERSION 3.22)
project(parallel-programming)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Wconversion -pedantic -Wfatal-errors -march=native)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W3 /WX)
endif()

add_subdirectory(in-class-1)
add_subdirectory(homework-1)
add_subdirectory(in-class-2)
add_subdirectory(homework-2)
add_subdirectory(in-class-3)
add_subdirectory(homework-3)
add_subdirectory(in-class-4)
add_subdirectory(homework-4)
add_subdirectory(in-class-5)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)