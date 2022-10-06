project(AlabasterView)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE sources ab_pch.hpp include/**.hpp src/**.cpp)

find_program(CLANG_FORMAT "clang-format")

if(CLANG_FORMAT)
    set(formattable_files ${sources})

    list(FILTER formattable_files)
    add_custom_target(clang-format COMMAND ${CLANG_FORMAT} -i -style=file
        ${formattable_files})
endif()

add_library(${PROJECT_NAME})

add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_dependencies(${PROJECT_NAME} clang-format)
