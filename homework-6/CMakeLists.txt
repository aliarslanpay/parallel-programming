project(homework-6)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-mavx -fstrict-overflow -fno-tree-vectorize)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/arch:AVX)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} parallel_implementation.cpp)
