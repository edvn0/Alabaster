cmake_minimum_required(VERSION 3.21)

project(AlabasterView)

find_package(Vulkan REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

find_package (Python3 COMPONENTS Interpreter)

file(GLOB_RECURSE sources av_pch.hpp Alabaster.hpp include/**.hpp src/**.cpp)

message(STATUS "Alabaster chosen OS: ${ALABASTER_OS}")

if(ALABASTER_OS STREQUAL "MacOS")
  list(FILTER sources EXCLUDE REGEX Windows*)
  list(APPEND sources
       ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/MacOS/MacOSWindow.cpp)
else()
  list(FILTER sources EXCLUDE REGEX MacOS*)
endif()

find_program(CLANG_FORMAT "clang-format")

if(CLANG_FORMAT)
  set(formattable_files ${sources})

  add_custom_target(clang-format-lib COMMAND ${CLANG_FORMAT} -i -style=file
                                             ${formattable_files})
endif()

add_library(${PROJECT_NAME} ${sources})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set(THIRD_PARTY_DIR ../../third_party/)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> ${THIRD_PARTY_DIR}/glm
  INTERFACE ${Vulkan_INCLUDE_DIRS}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          /opt/homebrew/include
          /usr/local/include
          ${THIRD_PARTY_DIR}/imgui
          ${THIRD_PARTY_DIR}/imgui/backends
          ${THIRD_PARTY_DIR}/debugbreak
          ${THIRD_PARTY_DIR}/magic_enum/include
          ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include
          ${THIRD_PARTY_DIR}/stb
          ${THIRD_PARTY_DIR}/tinyobjloader
          ${THIRD_PARTY_DIR}/random/include)

target_link_libraries(
  ${PROJECT_NAME}
  INTERFACE Boost::program_options
  PRIVATE glfw
          imgui
          Vulkan::Vulkan
          spdlog
          magic_enum
          VulkanMemoryAllocator
          tinyobjloader
          effolkronium_random)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_precompile_headers(${PROJECT_NAME} PUBLIC include/av_pch.hpp)

if(APPLE)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-volatile)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_VALIDATION)
  target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL
                                              "RelWithDebInfo")
  target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_RELEASE)
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

add_custom_target(
  compile_all_shaders ALL
  COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/compile_shaders.py --dir ${CMAKE_SOURCE_DIR}/app/resources/shaders --compiler
    "/usr/local/bin/glslc" --cache-name shaders --output-dir ${CMAKE_SOURCE_DIR}/app/resources/shaders
  COMMENT "Compiling shaders.")

add_dependencies(${PROJECT_NAME} compile_all_shaders)

add_dependencies(${PROJECT_NAME} clang-format-lib)
