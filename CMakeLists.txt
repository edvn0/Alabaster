cmake_minimum_required(VERSION 3.21)

project(
    Alabaster
    VERSION 0.0.1
    LANGUAGES CXX)

if(WIN32)
    list(APPEND CMAKE_MODULE_PATH "C:/Development")
    list(APPEND CMAKE_MODULE_PATH "D:/Development")

    set(BOOST_DIR "D:/Development/boost/boost_1_79_0")
    set(BOOST_ROOT "D:/Development/boost/boost_1_79_0")
    set(BOOST_LIBRARYDIR "D:/Development/boost/boost_1_79_0/stage/lib")
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    list(APPEND CMAKE_MODULE_PATH ${BOOST_LIBRARYDIR})
    list(APPEND CMAKE_MODULE_PATH ${BOOST_DIR})
    list(APPEND CMAKE_MODULE_PATH ${BOOST_ROOT})

    set(FT_DISABLE_HARFBUZZ ON)
    set(ALABASTER_OS "Windows")
else()
    set(ALABASTER_OS "MacOS")
endif()

if(CMAKE_GENERATOR STREQUAL Xcode)
    message(STATUS ${CMAKE_SOURCE_DIR})
    set(XCODE_SCHEME_ENABLE_GPU_API_VALIDATION ON)
    set(XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set(XCODE_SCHEME_ENABLE_GPU_SHADER_VALIDATION ON)
endif()

set(CMAKE_SUPPRESS_REGENERATION ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

include(CTest)

add_subdirectory(third_party)
add_subdirectory(libs)
add_subdirectory(app)
