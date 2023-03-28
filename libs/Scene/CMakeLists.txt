cmake_minimum_required(VERSION 3.21)
include("${CMAKE_SOURCE_DIR}/cmake/common_alabaster.cmake")

project(SceneSystem)

file(GLOB_RECURSE sources scene_pch.hpp Scene.hpp include/**.hpp src/**.cpp)

add_library(${PROJECT_NAME} ${sources})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
	${PROJECT_NAME}
	PUBLIC $<INSTALL_INTERFACE:include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	${THIRD_PARTY_DIR}
	${THIRD_PARTY_DIR}/glm
	${THIRD_PARTY_DIR}/spdlog/include
	${THIRD_PARTY_DIR}/imgui
	${THIRD_PARTY_DIR}/debugbreak
	${THIRD_PARTY_DIR}/magic_enum/include
	${THIRD_PARTY_DIR}/entt/single_include
	${THIRD_PARTY_DIR}/json/single_include
	${THIRD_PARTY_DIR}/stduuid/include
    ${THIRD_PARTY_DIR}/glfw/include
    ${THIRD_PARTY_DIR}/imguizmo
    ${THIRD_PARTY_DIR}/imgui
    ${THIRD_PARTY_DIR}/gsl/include
	../Core/include/
	../AssetManager/include/
	INTERFACE ${Vulkan_INCLUDE_DIRS}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE Vulkan::Vulkan
	spdlog
	magic_enum
	effolkronium_random
	imgui
    imguizmo
	nlohmann_json
	stduuid
	Alabaster::Core
	Alabaster::AssetManager)

target_precompile_headers(${PROJECT_NAME} PUBLIC include/scene_pch.hpp)
default_register_project(${PROJECT_NAME})