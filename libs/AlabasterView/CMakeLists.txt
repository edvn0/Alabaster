cmake_minimum_required(VERSION 3.21)

project(AlabasterView)

find_package(Vulkan REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

file(GLOB_RECURSE sources av_pch.hpp Alabaster.hpp include/**.hpp src/**.cpp)

find_program(CLANG_FORMAT "clang-format")

if(CLANG_FORMAT)
    set(formattable_files ${sources})

    add_custom_target(clang-format COMMAND ${CLANG_FORMAT} -i -style=file
        ${formattable_files})
endif()

add_library(${PROJECT_NAME} ${sources})
add_library(Alabaster::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    INTERFACE ${Vulkan_INCLUDE_DIRS}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} /opt/homebrew/include /usr/local/include ../../third_party/imgui ../../third_party/imgui/backends ../../third_party/debugbreak ../../third_party/magic_enum/include)

target_link_libraries(${PROJECT_NAME} INTERFACE Boost::program_options PRIVATE glfw imgui Vulkan::Vulkan spdlog magic_enum)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_precompile_headers(${PROJECT_NAME} PUBLIC include/av_pch.hpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-volatile)

if(${ALABASTER_VALIDATION} STREQUAL "ON")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_VALIDATION)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_dependencies(${PROJECT_NAME} clang-format)
