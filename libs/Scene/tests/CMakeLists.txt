cmake_minimum_required(VERSION 3.14)
project(SceneTests)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

file(GLOB_RECURSE sources main.cpp)

add_executable(SceneTests ${sources})
target_link_libraries(SceneTests gmock_main ${GTEST_LIBRARIES} Alabaster::SceneSystem Alabaster::Core)

include(GoogleTest)
gtest_discover_tests(SceneTests)
