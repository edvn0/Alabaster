cmake_minimum_required(VERSION 3.14)
project(AssetManagerTests)

set(CMAKE_CXX_STANDARD 20)
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")

file(GLOB_RECURSE sources compiler/**.cpp)

if(MSVC)
  list(APPEND sources "utils/msvc/LeakDetector.cpp")
else()
  list(APPEND sources "utils/LeakDetector.cpp")
endif()

add_executable(AssetManagerTests ${sources})
target_include_directories(
  AssetManagerTests PRIVATE "${THIRD_PARTY_DIR}/googletest/googletest/include"
                            "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(AssetManagerTests gmock_main GTest::gtest_main
                      Alabaster::AssetManager Alabaster::Core)

include(GoogleTest)
gtest_discover_tests(AssetManagerTests)
