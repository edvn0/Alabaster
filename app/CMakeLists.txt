cmake_minimum_required(VERSION 3.21)

project(AlabasterApp)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../third_party/spdlog/include)
target_link_libraries(${PROJECT_NAME} PRIVATE Alabaster::AlabasterView spdlog)
