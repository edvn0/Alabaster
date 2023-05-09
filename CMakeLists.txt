cmake_minimum_required(VERSION 3.21)

project(
  Alabaster
  VERSION 0.0.1
  LANGUAGES CXX)

find_package(Vulkan REQUIRED)
find_program(CLANG_FORMAT "clang-format")
find_package(Python COMPONENTS Development Interpreter)

if(WIN32)
  set(FT_DISABLE_HARFBUZZ ON)
  set(ALABASTER_OS "Windows")

  if(MINGW)
    set(ALABASTER_COMPILER "MinGW")
  elseif(MSVC)
    set(ALABASTER_COMPILER "MSVC")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 PROPERTY VS_STARTUP_PROJECT AlabasterApp)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(ALABASTER_COMPILER "Clang")
  else()
    message(FATAL_ERROR "Only MinGW (g++), MSVC and Clang are supported")
  endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(ALABASTER_OS "MacOS")
  set(ALABASTER_COMPILER "Clang")
else()
  set(ALABASTER_OS "Linux")
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(ALABASTER_COMPILER "Clang")
  else()
    set(ALABASTER_COMPILER "GNU")
  endif()
endif()

set(THIRD_PARTY_BUILDS "${CMAKE_SOURCE_DIR}/third_party_builds")
set(COMPILER_BUILD_TYPE
    "${ALABASTER_OS}-${ALABASTER_COMPILER}-${CMAKE_BUILD_TYPE}")
set(EXTRA_LIBRARY_DIR "${THIRD_PARTY_BUILDS}/${COMPILER_BUILD_TYPE}/lib")

message(
  STATUS
    "Alabaster: Chosen OS: ${ALABASTER_OS}
  Alabaster: Chosen compiler: ${ALABASTER_COMPILER}.
  Alabaster: Test config: ${BUILD_TESTING}, ${ALABASTER_BUILD_TESTING}.
  Alabaster: Prebuilt binary paths: ${EXTRA_LIBRARY_DIR}")

if(CMAKE_GENERATOR STREQUAL "Xcode")
  set(XCODE_SCHEME_ENABLE_GPU_API_VALIDATION ON)
  set(XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  set(XCODE_SCHEME_ENABLE_GPU_SHADER_VALIDATION ON)
endif()

set(CMAKE_SUPPRESS_REGENERATION ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Shaderc CMake legacy fix
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0042 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0079 NEW)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

include(CTest)
add_subdirectory(third_party)
add_subdirectory(libs)
add_subdirectory(app)
