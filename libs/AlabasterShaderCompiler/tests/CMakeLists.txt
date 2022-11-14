cmake_minimum_required(VERSION 3.14)
project(AlabasterShaderCompilerTests)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

file(GLOB_RECURSE sources main.cpp compiler/main.cpp)

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} GTest::gtest_main AlabasterShaderCompiler)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
