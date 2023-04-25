set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")

function(register_for_project PROJECT HAS_TESTS)
	if(CLANG_FORMAT)
		set(formattable_files ${sources})

		add_custom_target(
			"clang-format-${PROJECT_NAME}" COMMAND ${CLANG_FORMAT} -i -style=file
			${formattable_files})
	endif()

	if(${ALABASTER_OS} STREQUAL "Windows")
		target_compile_definitions(${PROJECT} PRIVATE ALABASTER_WINDOWS)
	elseif(${ALABASTER_OS} STREQUAL "MacOS")
		target_compile_definitions(${PROJECT} PRIVATE ALABASTER_MACOS)
	elseif(${ALABASTER_OS} STREQUAL "Linux")
		target_compile_definitions(${PROJECT} PRIVATE ALABASTER_LINUX)
	endif()

	target_compile_features(${PROJECT} PRIVATE cxx_std_20)

	if(MSVC)
		target_compile_options(${PROJECT} PRIVATE /W4 /WX /wd4201)
	else()
		target_compile_options(
			${PROJECT} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Werror
			-Wno-nullability-extension)
	endif()

    if(NOT ${ALABASTER_OS} STREQUAL "Windows")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
        set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
        set_property(TARGET fmt PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()

	if ("${Python_FOUND}")
		target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_HAS_PYTHON)
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_definitions(${PROJECT} PRIVATE ALABASTER_VALIDATION)
		target_compile_definitions(${PROJECT} PRIVATE ALABASTER_DEBUG)
		target_compile_definitions(${PROJECT} PRIVATE ALABASTER_EXCEPTIONS)
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL
		"RelWithDebInfo")
		target_compile_definitions(${PROJECT} PRIVATE ALABASTER_PROFILE)
		target_compile_definitions(${PROJECT} PRIVATE ALABASTER_RELEASE)
	endif()

	if(MSVC)
		target_compile_options(${PROJECT} PRIVATE "/MP")
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "AlabasterApp")
	endif()

	if(MINGW)
		target_link_libraries(${PROJECT} PRIVATE -static-libgcc -static-libstdc++)
	endif()

	if(CLANG_FORMAT)
		add_dependencies(${PROJECT_NAME} "clang-format-${PROJECT_NAME}")
	endif()

    if(${ALABASTER_BUILD_TESTING})
        add_subdirectory(tests)
    endif()
endfunction()

function(default_register_project PROJECT)
	register_for_project(${PROJECT} ON)
endfunction()

function(add_os_specific_objects sources all_sources)
	set(_ALL_INPUTS ${${sources}})
	file(GLOB_RECURSE private_os_sources "src/platform/${ALABASTER_OS}/**.cpp")
	file(GLOB_RECURSE compiler_sources "src/platform/${ALABASTER_COMPILER}/**.cpp")
	list(APPEND _ALL_INPUTS ${private_os_sources} ${compiler_sources})
	set(${all_sources} ${_ALL_INPUTS} PARENT_SCOPE)
endfunction(add_os_specific_objects)
