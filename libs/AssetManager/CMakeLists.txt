cmake_minimum_required(VERSION 3.21)
include("${CMAKE_SOURCE_DIR}/cmake/common_alabaster.cmake")

project(AssetManager)

file(GLOB_RECURSE sources am_pch.hpp AssetManager.hpp include/**.hpp "src/cache/**.cpp" "src/watcher/**.cpp" "src/compiler/**.cpp")
set(input_sources "${sources}")
set(all_sources "")
add_os_specific_objects(input_sources all_sources)

add_library(${PROJECT_NAME} ${all_sources})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
	${PROJECT_NAME}
	PUBLIC $<INSTALL_INTERFACE:include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	${THIRD_PARTY_DIR}
	${THIRD_PARTY_DIR}/glm
	${THIRD_PARTY_DIR}/spdlog/include
	${THIRD_PARTY_DIR}/debugbreak
	${THIRD_PARTY_DIR}/magic_enum/include
    ${THIRD_PARTY_DIR}/gsl/include
	../Core/include/
	INTERFACE ${Vulkan_INCLUDE_DIRS}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE Vulkan::Vulkan
	spdlog
	magic_enum
	effolkronium_random
	Alabaster::Core
	shaderc
    spirv-cross-core
    spirv-cross-glsl
)

target_precompile_headers(${PROJECT_NAME} PUBLIC include/am_pch.hpp)
default_register_project(${PROJECT_NAME})
