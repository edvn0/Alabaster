#!/bin/bash

current_dir="."

cmake -B build -GNinja \
    -DGLFW_INSTALL=OFF \
    -DGLFW_BUILD_DOCS=OFF \
    -DGLFW_BUILD_TESTS=OFF \
    -DGLFW_BUILD_EXAMPLES=OFF \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=OFF \
    -DALABASTER_VALIDATION=ON \
    -DBUILD_SHARED_LIBS=OFF -S "$current_dir"

cmake --build build
rm "$current_dir/compile_commands.json"
ln -s build/compile_commands.json "$current_dir"

pwd
exec ./build/app/AlabasterApp "$@"
