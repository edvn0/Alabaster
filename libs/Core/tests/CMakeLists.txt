cmake_minimum_required(VERSION 3.14)
project(CoreTests)

set(CMAKE_CXX_STANDARD 20)
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")

file(
	GLOB_RECURSE
	sources
	main.cpp
	core/**.cpp
	graphics/**.cpp
	utils/**.cpp
	platform/**.cpp)

add_executable(CoreTests ${sources})
target_include_directories(CoreTests PRIVATE "${THIRD_PARTY_DIR}/googletest/googletest/include")
target_link_libraries(CoreTests GTest::gtest_main Alabaster::Core)

include(GoogleTest)
gtest_discover_tests(CoreTests)
