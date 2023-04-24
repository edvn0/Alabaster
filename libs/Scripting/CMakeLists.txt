cmake_minimum_required(VERSION 3.21)
include("${CMAKE_SOURCE_DIR}/cmake/common_alabaster.cmake")

project(Scripting)

find_package(Python COMPONENTS Interpreter Development)

file(GLOB_RECURSE sources include/script_pch.hpp include/Scripting.hpp include/**.hpp src/engine/**.cpp)

if(${Python_FOUND})
    list(APPEND sources "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/python/PythonScriptingEngine.cpp")
endif()

add_library(${PROJECT_NAME} ${sources})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${CMAKE_SOURCE_DIR}/libs/Core/include/
    ${CMAKE_SOURCE_DIR}/libs/Scene/include/
    INTERFACE 
    ${Vulkan_INCLUDE_DIRS}
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    pybind11::pybind11
    Alabaster::Core
    Alabaster::SceneSystem
    Python::Python)

if("${ALABASTER_BUILD_TESTING}")
    add_subdirectory(tests)
endif()

target_precompile_headers(${PROJECT_NAME} PUBLIC include/script_pch.hpp)
default_register_project(${PROJECT_NAME})


# ----------- PYBIND-MODULE ----------- #
pybind11_add_module(alabaster_entity MODULE src/platform/python_bindings/PythonEntity.cpp)
target_include_directories(alabaster_entity PRIVATE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set_property(TARGET alabaster_entity PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(alabaster_entity PRIVATE
    Alabaster::SceneSystem
    Alabaster::Core
    pybind11::pybind11
)

add_custom_command(
	TARGET alabaster_entity
	POST_BUILD
	COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:alabaster_entity> $<TARGET_FILE_DIR:AlabasterApp>/resources/scripts
)

add_custom_command(
	TARGET alabaster_entity
	POST_BUILD
	COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:alabaster_entity> ${CMAKE_SOURCE_DIR}/app/resources/scripts
)
# ----------- PYBIND-MODULE ----------- #
