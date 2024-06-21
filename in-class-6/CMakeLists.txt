project(in-class-6)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-mavx)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/arch:AVX)
endif()

find_package(MPI REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${MPI_INCLUDE_PATH})

add_executable(${PROJECT_NAME} parallel_implementation.cpp)

target_link_libraries(${PROJECT_NAME} ${MPI_LIBRARIES})