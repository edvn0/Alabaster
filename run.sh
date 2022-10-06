#!/bin/bash

cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="$(which clang++)" -DCMAKE_C_COMPILER="$(which clang)" -DCMAKE_CXX_STANDARD=20 -S . 
cmake --build build
exec ./build/app/app
