#!/usr/bin/env bash

set -e

current_dir="."
build_folder="build"

should_run=${1:OFF}
build_testing=${2:OFF}
clean_first=${3:OFF}
build_type=${4:Debug}

cmake -B "$build_folder" \
    -G "Visual Studio 17 2022" \
    -D GLFW_INSTALL=OFF \
    -D GLFW_BUILD_DOCS=OFF \
    -D GLFW_BUILD_TESTS=OFF \
    -D GLFW_BUILD_EXAMPLES=OFF \
    -D BUILD_EXAMPLES=OFF \
    -D CMAKE_BUILD_TYPE="$build_type" \
    -D CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug \
    -D BUILD_TESTING="$build_testing" \
    -D Random_BuildTests="OFF" \
    -D BUILD_SHARED_LIBS=OFF \
    -S "$current_dir"

if [ "$clean_first" = "ON" ]; then
    cmake --build "$build_folder" --clean-first
else
    cmake --build "$build_folder"
fi

run_tests() {
    ctest -j10 --test-dir "build"
}

run_app() {
    pushd "$build_folder/app/Debug" || exit
    "./AlabasterApp" "$@"
    popd || exit
}

run_tests
shift 1
if [ "$should_run" = "ON" ]; then
    run_app "$@"
fi
