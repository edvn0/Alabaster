#!/bin/bash

cmake -B build -GNinja -DBUILD_TESTING=OFF -S .
cmake --build build
rm compile_commands.json
ln -s build/compile_commands.json .
exec ./build/app/AlabasterApp
