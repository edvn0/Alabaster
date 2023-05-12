cmake_minimum_required(VERSION 3.14)
project(SceneTests)

set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE sources compiler/**.cpp)

add_executable(SceneTests ${sources})
target_include_directories(
  SceneTests PRIVATE "${THIRD_PARTY_DIR}/googletest/googletest/include")
target_link_libraries(SceneTests gmock_main GTest::gtest_main
                      Alabaster::SceneSystem Alabaster::Core)

include(GoogleTest)
gtest_discover_tests(SceneTests)
