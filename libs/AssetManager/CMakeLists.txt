cmake_minimum_required(VERSION 3.21)
include("${CMAKE_SOURCE_DIR}/cmake/common_alabaster.cmake")

project(AssetManager)

file(
  GLOB_RECURSE
  sources
  am_pch.hpp
  AssetManager.hpp
  include/**.hpp
  "src/cache/**.cpp"
  "src/watcher/**.cpp"
  "src/compiler/**.cpp")
set(INPUT_SOURCES "${sources}")
set(ALL_SOURCES "")
add_os_specific_objects(INPUT_SOURCES ALL_SOURCES)

add_library(${PROJECT_NAME} ${ALL_SOURCES})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<INSTALL_INTERFACE:include>
         ${CMAKE_CURRENT_SOURCE_DIR}
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         ${THIRD_PARTY_DIR}
         ${THIRD_PARTY_DIR}/glm
         ${THIRD_PARTY_DIR}/debugbreak
         ${THIRD_PARTY_DIR}/magic_enum/include
         ${THIRD_PARTY_DIR}/gsl/include
  INTERFACE ${Vulkan_INCLUDE_DIRS}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE Vulkan::Vulkan
          magic_enum
          effolkronium_random
          Alabaster::Core
          shaderc
          spirv-cross-core
          spirv-cross-glsl)

target_precompile_headers(${PROJECT_NAME} PUBLIC include/am_pch.hpp)
default_register_project(${PROJECT_NAME})
