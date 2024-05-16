project(in-class-3)

find_package(OpenMP REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} parallel_implementation.cpp)
target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
