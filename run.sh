#!/bin/bash

cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF -DALABASTER_VALIDATION=ON -DBUILD_SHARED_LIBS=OFF -S .
cmake --build build
rm compile_commands.json
ln -s build/compile_commands.json .
exec ./build/app/AlabasterApp
