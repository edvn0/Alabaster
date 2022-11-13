cmake_minimum_required(VERSION 3.14)
project(AlabasterViewTests)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

file(
  GLOB_RECURSE
  sources
  main.cpp
  core/**.cpp
  graphics/**.cpp
  utils/**.cpp
  platform/**.cpp)

add_executable(AlabasterViewTests ${sources})
target_link_libraries(AlabasterViewTests GTest::gtest_main AlabasterView)

include(GoogleTest)
gtest_discover_tests(AlabasterViewTests)
