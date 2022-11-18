cmake_minimum_required(VERSION 3.14)
project(AssetManagerTests)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

file(GLOB_RECURSE sources main.cpp compiler/**.cpp)

add_executable(AssetManagerTests ${sources})
target_link_libraries(AssetManagerTests gmock_main GTest::gtest_main Alabaster::AssetManager Alabaster::Core)

include(GoogleTest)
gtest_discover_tests(AssetManagerTests)
