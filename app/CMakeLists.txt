cmake_minimum_required(VERSION 3.21)

project(AlabasterApp)

set(THIRD_PARTY ../third_party)

file(GLOB_RECURSE sources main.cpp Alabaster.hpp AlabasterLayer.hpp
	AlabasterLayer.cpp include/**.hpp src/**.cpp)

find_program(CLANG_FORMAT "clang-format")

if(CLANG_FORMAT AND ${ALABASTER_IS_BUILD_TOOL} STREQUAL "OFF" AND ${ALABASTER_SHOULD_FORMAT} STREQUAL "ON")
	file(GLOB_RECURSE shaders resources/shaders/**.vert resources/shaders/**.frag)

	set(formattable_files ${sources})
	list(APPEND formattable_files ${shaders})

	add_custom_target(clang-format-app COMMAND ${CLANG_FORMAT} -i -style=file
		${formattable_files})
endif()

add_executable(${PROJECT_NAME} ${sources})

target_include_directories(
	${PROJECT_NAME}
	PRIVATE include ${THIRD_PARTY}/debugbreak ${THIRD_PARTY}/magic_enum/include
	${THIRD_PARTY}/glm ${THIRD_PARTY}/random/include)
target_link_libraries(
	${PROJECT_NAME} PRIVATE Alabaster::AssetManager Alabaster::Core
	Alabaster::SceneSystem Alabaster::Scripting)

if(MINGW)
	target_link_libraries(
		${PROJECT_NAME}
		PRIVATE -static-libgcc -static-libstdc++)
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

if(CLANG_FORMAT AND ${ALABASTER_IS_BUILD_TOOL} STREQUAL "OFF" AND ${ALABASTER_SHOULD_FORMAT} STREQUAL "ON")
	add_dependencies(${PROJECT_NAME} clang-format-app)
endif()

add_custom_command(
	TARGET ${PROJECT_NAME}
    PRE_BUILD
	COMMAND
	${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:AlabasterApp>/resources/
)
