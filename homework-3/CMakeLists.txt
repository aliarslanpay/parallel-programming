project(homework-3)

find_package(Boost 1.65 REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})
add_library(Utility OBJECT Utility.cpp)

add_executable(${PROJECT_NAME} parallel_implementation.cpp $<TARGET_OBJECTS:Utility>)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} -lcrypto)
