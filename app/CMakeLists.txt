cmake_minimum_required(VERSION 3.21)

project(AlabasterApp)

set(THIRD_PARTY ../third_party)

file(GLOB_RECURSE sources main.cpp Alabaster.hpp AlabasterLayer.hpp
	AlabasterLayer.cpp)

find_program(CLANG_FORMAT "clang-format")

if(CLANG_FORMAT)
	file(GLOB_RECURSE shaders resources/shaders/**.vert resources/shaders/**.frag)

	set(formattable_files ${sources})
	list(APPEND formattable_files ${shaders})

	add_custom_target(clang-format-app COMMAND ${CLANG_FORMAT} -i -style=file
		${formattable_files})
endif()

add_executable(${PROJECT_NAME} ${sources})

target_include_directories(
	${PROJECT_NAME}
	PRIVATE ${THIRD_PARTY}/debugbreak ${THIRD_PARTY}/magic_enum/include
	${THIRD_PARTY}/glm ${THIRD_PARTY}/random/include)
target_link_libraries(
	${PROJECT_NAME} PRIVATE Alabaster::AssetManager Alabaster::Core
	Alabaster::SceneSystem spdlog imgui)

if(WIN32)
	target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_WINDOWS /wd4201)
elseif(APPLE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_MACOS)
endif()

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MP" /wd4201)
endif()

add_custom_command(
	TARGET AlabasterApp
	POST_BUILD
	COMMAND
	${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources
	$<TARGET_FILE_DIR:AlabasterApp>/resources)

add_dependencies(${PROJECT_NAME} clang-format-app)
