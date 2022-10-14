cmake_minimum_required(VERSION 3.21)

project(AlabasterApp)

set(sources main.cpp AlabasterLayer.cpp)

find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
  set(formattable_files ${sources})

  add_custom_target(clang-format-app COMMAND ${CLANG_FORMAT} -i -style=file
                                             ${formattable_files})
endif()

add_executable(${PROJECT_NAME} ${sources})

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ../third_party/spdlog/include ../third_party/imgui
          ../third_party/debugbreak ../third_party/magic_enum/include
          ../third_party/VulkanMemoryAllocator/include ../third_party/glm)
target_link_libraries(${PROJECT_NAME} PRIVATE Alabaster::AlabasterView spdlog
                                              imgui)

add_custom_command(
  TARGET AlabasterApp
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:AlabasterApp>/resources)
