project(homework-7)

find_package(MPI REQUIRED)

set(SOURCES
        parallel_implementation.cpp
        Utility.cpp
        VideoOutput.cpp
        lib/TinyPngOut.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${MPI_INCLUDE_PATH})

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${MPI_LIBRARIES})