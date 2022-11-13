cmake_minimum_required(VERSION 3.21)

project(AlabasterApp)

file(GLOB_RECURSE sources main.cpp Alabaster.hpp AlabasterLayer.hpp AlabasterLayer.cpp)

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
    PRIVATE ../third_party/spdlog/include
    ../third_party/imgui
    ../third_party/debugbreak
    ../third_party/magic_enum/include
    ../third_party/VulkanMemoryAllocator/include
    ../third_party/glm
    ../third_party/random/include)
target_link_libraries(${PROJECT_NAME} PRIVATE Alabaster::AlabasterView spdlog
    imgui)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ALABASTER_MACOS)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
endif()

add_custom_command(
    TARGET AlabasterApp
    POST_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:AlabasterApp>/resources)

add_dependencies(${PROJECT_NAME} clang-format-app)