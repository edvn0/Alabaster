#!/bin/bash

current_dir="."

build_testing=${1:OFF}

cmake -B build -GNinja \
    -DGLFW_INSTALL=OFF \
    -DGLFW_BUILD_DOCS=OFF \
    -DGLFW_BUILD_TESTS=OFF \
    -DGLFW_BUILD_EXAMPLES=OFF \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING="$build_testing" \
    -DBUILD_SHARED_LIBS=OFF -S "$current_dir"

cmake --build build
rm "$current_dir/compile_commands.json"
ln -s build/compile_commands.json "$current_dir"

run_tests() {
  pushd build || exit
  ctest
  popd || exit
}

run_app() {
  exec ./build/app/AlabasterApp "$@"
}

run_tests
shift 1
run_app "$@"
