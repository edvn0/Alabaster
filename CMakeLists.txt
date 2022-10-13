cmake_minimum_required(VERSION 3.21)

project(
  Alabaster
  VERSION 0.0.1
  LANGUAGES CXX)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

include(CTest)

add_subdirectory(third_party)
add_subdirectory(libs)
add_subdirectory(app)
