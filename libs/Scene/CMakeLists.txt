cmake_minimum_required(VERSION 3.21)

project(SceneSystem)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE sources scene_pch.hpp Scene.hpp include/**.hpp src/**.cpp)

find_program(CLANG_FORMAT "clang-format")

if(CLANG_FORMAT)
	set(formattable_files ${sources})

	add_custom_target(clang-format-scene COMMAND ${CLANG_FORMAT} -i -style=file
		${formattable_files})
endif()

add_library(${PROJECT_NAME} ${sources})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set(THIRD_PARTY_DIR ../../third_party/)

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
	nlohmann_json
	stduuid
	Alabaster::Core
	Alabaster::AssetManager)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_precompile_headers(${PROJECT_NAME} PUBLIC include/scene_pch.hpp)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4201)
else()
	target_compile_options(
		${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-shadow -Werror
		-Wno-nullability-extension)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_VALIDATION)
	target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL
	"RelWithDebInfo")
	target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_PROFILE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_RELEASE)
endif()

if(${ALABASTER_OS} STREQUAL "Windows")
	target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_WINDOWS)
elseif(${ALABASTER_OS} STREQUAL "MacOS")
	target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_MACOS)
elseif(${ALABASTER_OS} STREQUAL "Linux")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_LINUX)
endif()

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
endif()

add_subdirectory(tests)

if(CLANG_FORMAT)
	add_dependencies(${PROJECT_NAME} clang-format-scene)
endif()
