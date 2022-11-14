cmake_minimum_required(VERSION 3.21)

project(AlabasterShaderCompiler)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE sources asc_pch.hpp AlabasterShaderCompiler.hpp
     include/**.hpp src/**.cpp)

find_program(CLANG_FORMAT "clang-format")

if(CLANG_FORMAT)
  set(formattable_files ${sources})

  add_custom_target(clang-format-shader-compiler
                    COMMAND ${CLANG_FORMAT} -i -style=file ${formattable_files})
endif()

add_library(${PROJECT_NAME} ${sources})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set(THIRD_PARTY_DIR ../../third_party/)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         ${THIRD_PARTY_DIR}/glm
         ${THIRD_PARTY_DIR}/spdlog/include
         ${THIRD_PARTY_DIR}/debugbreak
         ${THIRD_PARTY_DIR}/magic_enum/include
         ../AlabasterView/include/
  INTERFACE ${Vulkan_INCLUDE_DIRS}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} /opt/homebrew/include /usr/local/include)

target_link_libraries(
  ${PROJECT_NAME} PRIVATE Vulkan::Vulkan spdlog magic_enum effolkronium_random
                          AlabasterView shaderc)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_precompile_headers(${PROJECT_NAME} PUBLIC include/asc_pch.hpp)

if(APPLE)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-volatile)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_VALIDATION)
  target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL
                                              "RelWithDebInfo")
  target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_PROFILE)
  target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_RELEASE)
endif()

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_WINDOWS)
elseif(APPLE)
  target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_MACOS)
endif()

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

add_dependencies(${PROJECT_NAME} clang-format-shader-compiler)
