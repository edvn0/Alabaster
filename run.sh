#!/bin/bash

set -e

current_dir="."
build_folder="build"

build_testing=${1:OFF}
clean_first=${2:OFF}

cmake -B "$build_folder" \
    -GNinja \
    -DGLFW_INSTALL=OFF \
    -DGLFW_BUILD_DOCS=OFF \
    -DGLFW_BUILD_TESTS=OFF \
    -DGLFW_BUILD_EXAMPLES=OFF \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING="$build_testing" \
    -DBUILD_SHARED_LIBS=OFF \
    -S "$current_dir"

if [ "$clean_first" = "ON" ];
then
  cmake --build "$build_folder" --clean-first
else
  cmake --build "$build_folder"
fi;

rm "$current_dir/compile_commands.json"
ln -s build/compile_commands.json "$current_dir"

run_tests() {
    ctest -j10 --test-dir "build"
}

run_app() {
    exec "./$build_folder/app/AlabasterApp" "$@"
}

run_tests
shift 1
run_app "$@"
