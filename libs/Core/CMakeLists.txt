cmake_minimum_required(VERSION 3.21)
include("${CMAKE_SOURCE_DIR}/cmake/common_alabaster.cmake")

project(Core)

file(
  GLOB_RECURSE
  sources
  av_pch.hpp
  Alabaster.hpp
  "src/core/**.cpp"
  "src/graphics/**.cpp"
  "src/ui/**.cpp"
  "src/utilities/**.cpp"
  "src/platform/stb/**.cpp"
  "src/platform/Vulkan/**.cpp"
  "src/platform/ImGui/**.cpp")
set(INPUT_SOURCE "${sources}")
set(ALL_SOURCES "")
add_os_specific_objects(INPUT_SOURCE ALL_SOURCES)

add_library(${PROJECT_NAME} ${ALL_SOURCES})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         ${THIRD_PARTY_DIR}/glm
         ${THIRD_PARTY_DIR}/ProgramOptions/include
         ${THIRD_PARTY_DIR}/spdlog/include
         ${THIRD_PARTY_DIR}/fmt/include
         ${THIRD_PARTY_DIR}/debugbreak
         ${THIRD_PARTY_DIR}/magic_enum/include
         ${THIRD_PARTY_DIR}/random/include
         ${THIRD_PARTY_DIR}/stduuid/include
         ${THIRD_PARTY_DIR}/imgui
         ${THIRD_PARTY_DIR}/imguizmo
         ${THIRD_PARTY_DIR}/imgui/backends
         ${THIRD_PARTY_DIR}/stb
         ${THIRD_PARTY_DIR}/tinyobjloader
         ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include
         ${THIRD_PARTY_DIR}/gsl/include
  INTERFACE ${Vulkan_INCLUDE_DIRS}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE glfw
          imguizmo
          Vulkan::Vulkan
          magic_enum
          ProgramOptionsHxx
          VulkanMemoryAllocator
          tinyobjloader
          effolkronium_random
          Alabaster::AssetManager
  PUBLIC spdlog imgui fmt::fmt)

target_precompile_headers(${PROJECT_NAME} PUBLIC include/av_pch.hpp)
default_register_project(${PROJECT_NAME})
