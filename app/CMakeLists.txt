cmake_minimum_required(VERSION 3.21)

project(AlabasterApp)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Alabaster::AlabasterView)
