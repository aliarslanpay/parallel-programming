project(homework-5)

find_package(OpenMP REQUIRED)

set(SOURCES
        parallel_implementation.cpp
        Utility.cpp
        VideoOutput.cpp
        lib/lodepng.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
