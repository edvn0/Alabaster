cmake_minimum_required(VERSION 3.21)
include("${CMAKE_SOURCE_DIR}/cmake/common_alabaster.cmake")

project(Scripting)

find_package(Python COMPONENTS Interpreter Development)

file(GLOB_RECURSE sources include/script_pch.hpp include/Scripting.hpp include/**.hpp src/engine/**.cpp)

add_library(${PROJECT_NAME} ${sources})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${CMAKE_SOURCE_DIR}/Core/include/
    ${CMAKE_SOURCE_DIR}/Scene/include/
    INTERFACE ${Vulkan_INCLUDE_DIRS}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    ${Python_INCLUDE_DIRS})

if(not ${ALABASTER_WINDOWS})
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
endif()

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    Alabaster::Core
    pybind11::pybind11
    Alabaster::SceneSystem
    Python::Python)

if("${ALABASTER_BUILD_TESTING}")
    add_subdirectory(tests)
endif()

target_precompile_headers(${PROJECT_NAME} PUBLIC include/script_pch.hpp)
default_register_project(${PROJECT_NAME})


# ----------- PYBIND-MODULE ----------- #
pybind11_add_module(alabaster_entity MODULE src/platform/python/PythonEntity.cpp)
target_include_directories(alabaster_entity PRIVATE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set_property(TARGET alabaster_entity PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(alabaster_entity PRIVATE
    Alabaster::Core
    Alabaster::SceneSystem
    pybind11::pybind11
)
add_custom_command(
	TARGET alabaster_entity
	POST_BUILD
	COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/Debug $<TARGET_FILE_DIR:AlabasterApp>/resources/scripts
)
# ----------- PYBIND-MODULE ----------- #
