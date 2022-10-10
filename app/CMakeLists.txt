cmake_minimum_required(VERSION 3.21)

project(AlabasterApp)

find_program(CLANG_FORMAT "clang-format")


add_executable(${PROJECT_NAME} main.cpp AlabasterLayer.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../third_party/spdlog/include ../third_party/imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE Alabaster::AlabasterView spdlog imgui)
