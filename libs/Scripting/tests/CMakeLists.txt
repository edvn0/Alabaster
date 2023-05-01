cmake_minimum_required(VERSION 3.14)
project(ScriptingTests)

set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE sources main.cpp interpreter_tests.cpp)

add_executable(ScriptingTests ${sources})
target_include_directories(ScriptingTests PRIVATE 
    "${THIRD_PARTY_DIR}/googletest/googletest/include"
)
target_link_libraries(ScriptingTests
    PRIVATE
    Alabaster::Core
    Alabaster::SceneSystem
    Alabaster::Scripting
    VulkanMemoryAllocator
    gmock_main 
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ScriptingTests)
