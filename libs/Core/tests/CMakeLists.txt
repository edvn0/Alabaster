cmake_minimum_required(VERSION 3.14)
project(CoreTests)

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

add_executable(CoreTests ${sources})
target_link_libraries(CoreTests ${GTEST_LIBRARIES} Alabaster::Core)

include(GoogleTest)
gtest_discover_tests(CoreTests)
