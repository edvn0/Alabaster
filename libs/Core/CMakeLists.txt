cmake_minimum_required(VERSION 3.21)
include("${CMAKE_SOURCE_DIR}/cmake/common_alabaster.cmake")

project(Core)

find_package(Vulkan REQUIRED)
find_package(Python3 COMPONENTS Interpreter)

file(GLOB_RECURSE sources av_pch.hpp Alabaster.hpp include/**.hpp src/**.cpp)

if(${ALABASTER_OS} STREQUAL "Windows")
    list(FILTER sources EXCLUDE REGEX MacOS*)
    list(FILTER sources EXCLUDE REGEX Linux*)
    list(FILTER sources EXCLUDE REGEX MSVC*)
    list(FILTER sources EXCLUDE REGEX MinGW*)

    if (ALABASTER_COMPILER STREQUAL "MinGW")
        file(GLOB_RECURSE compilerSpecificSources "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/MinGW/MinGWUI.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/MinGW/**.cpp")
        list(APPEND sources ${compilerSpecificSources})
        list(FILTER sources EXCLUDE REGEX MSVC)
    elseif(ALABASTER_COMPILER STREQUAL "MSVC")
        file(GLOB_RECURSE compilerSpecificSources "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/MSVC/MSVCUI.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/MSVC/**.cpp")
        list(APPEND sources ${compilerSpecificSources})
        list(FILTER sources EXCLUDE REGEX MinGW)
    endif()
elseif(${ALABASTER_OS} STREQUAL "MacOS")
    list(FILTER sources EXCLUDE REGEX Windows*)
    list(FILTER sources EXCLUDE REGEX Linux*)
    list(FILTER sources EXCLUDE REGEX MSVC*)
    list(FILTER sources EXCLUDE REGEX MinGW*)

    # Windows includes 'Window'...
    list(APPEND sources
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/MacOS/MacOSWindow.cpp)
elseif(${ALABASTER_OS} STREQUAL "Linux")
    list(FILTER sources EXCLUDE REGEX MacOS*)
    list(FILTER sources EXCLUDE REGEX Windows*)
    list(FILTER sources EXCLUDE REGEX MSVC*)
    list(FILTER sources EXCLUDE REGEX MinGW*)

    # Windows includes 'Window'...
    list(APPEND sources
        ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/Linux/LinuxWindow.cpp)
endif()

add_library(${PROJECT_NAME} ${sources})
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
	${THIRD_PARTY_DIR}/imgui
    ${THIRD_PARTY_DIR}/imguizmo
	${THIRD_PARTY_DIR}/imgui/backends
	${THIRD_PARTY_DIR}/stb
	${THIRD_PARTY_DIR}/tinyobjloader
	${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include
    ${THIRD_PARTY_DIR}/gsl/include
	INTERFACE ${Vulkan_INCLUDE_DIRS}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../AssetManager/include)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE glfw
	imgui
    imguizmo
	Vulkan::Vulkan
	spdlog
	magic_enum
	fmt::fmt
	ProgramOptionsHxx
	VulkanMemoryAllocator
	tinyobjloader
	effolkronium_random)

target_precompile_headers(${PROJECT_NAME} PUBLIC include/av_pch.hpp)
default_register_project(${PROJECT_NAME})
