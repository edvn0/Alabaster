#!/usr/bin/env bash

set -e

current_dir="."
build_folder="build"

should_run="OFF"
build_testing="OFF"
clean_first="OFF"
build_type="Debug"
generator="Ninja"

function alabaster_help() {
    echo "Usage: run [ -r <ON/*OFF> ] [ -t <ON/*OFF> ] [ -c <ON/*OFF> ] [ -b <*Debug/Release/RelWithDebInfo/MinSizeRel> ] [ -g <*Ninja/VS/<CMake Generator freetext>> ] [ -h ]"
}

while getopts g:r:t:c:b:h flag; do
    case "${flag}" in
    g) generator=${OPTARG} ;;
    r) should_run=${OPTARG} ;;
    t) build_testing=${OPTARG} ;;
    c) clean_first=${OPTARG} ;;
    b) build_type=${OPTARG} ;;
    h)
        alabaster_help
        exit 2
        ;;
    *)
        alabaster_help
        exit 2
        ;;
    esac
done

build_and_generator_folder="$build_folder-$build_type-$generator"

if [ "$clean_first" = "ON" ]; then
    rm -rf "$build_and_generator_folder/app"
    echo "Cleaned Alabaster::app folder"
    rm -rf "$build_and_generator_folder/libs"
    echo "Cleaned Alabaster::libs folder"
fi

if [ "$generator" = "VS" ]; then
    export CMAKE_GENERATOR="Visual Studio 17 2022"
elif [ "$generator" = "Ninja" ]; then
    export CMAKE_GENERATOR="Ninja"
else
    # This may just straight up fail
    export CMAKE_GENERATOR="$generator"
fi

cmake -B "$build_and_generator_folder" \
    -D GLFW_INSTALL=OFF \
    -D GLFW_BUILD_DOCS=OFF \
    -D GLFW_BUILD_TESTS=OFF \
    -D GLFW_BUILD_EXAMPLES=OFF \
    -D CMAKE_BUILD_TYPE="$build_type" \
    -D BUILD_TESTING="$build_testing" \
    -D ENABLE_HLSL=ON \
    -D ENABLE_CTEST=OFF \
    -D ENTT_BUILD_TESTING=OFF \
    -D Random_BuildTests=OFF \
    -D BUILD_SHARED_LIBS=OFF \
    -D SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=OFF \
    -D SPIRV_CROSS_SHARED=OFF \
    -D SPIRV_CROSS_STATIC=ON \
    -D SPIRV_CROSS_CLI=OFF \
    -D SPIRV_CROSS_ENABLE_TESTS=OFF \
    -D SPIRV_CROSS_ENABLE_GLSL=ON \
    -D SPIRV_CROSS_ENABLE_HLSL=OFF \
    -D SPIRV_CROSS_ENABLE_MSL=OFF \
    -D SPIRV_CROSS_ENABLE_CPP=OFF \
    -D SPIRV_CROSS_ENABLE_REFLECT=OFF \
    -D SPIRV_CROSS_ENABLE_C_API=OFF \
    -D SPIRV_CROSS_ENABLE_UTIL=OFF \
    -D SPIRV_CROSS_SKIP_INSTALL=ON \
    -D SHADERC_SKIP_INSTALL=ON \
    -D SHADERC_SKIP_TESTS=ON \
    -D SHADERC_SKIP_EXAMPLES=ON \
    -D SHADERC_SKIP_COPYRIGHT_CHECK=ON \
    -D SHADERC_ENABLE_WERROR_COMPILE=OFF \
    -D SPDLOG_FMT_EXTERNAL=ON \
    -D UUID_USING_CXX20_SPAN=ON \
    -S "$current_dir"

cmake --build "$build_and_generator_folder" --parallel 10

if [ "$generator" = "Ninja" ]; then
    if [ -f "$current_dir/compile_commands.json" ]; then
        rm "$current_dir/compile_commands.json"
    fi

    ln -s "$build_and_generator_folder/compile_commands.json" "$current_dir/compile_commands.json"
fi

run_tests() {
    if [ "$build_type" == "Debug" ] && [ "$build_testing" == "ON" ]; then
        ctest -j10 -C Debug --test-dir "$build_and_generator_folder"
    fi
}

run_app() {
    local appName="AlabasterApp"
    if [ "$OSTYPE" == "cygwin" ] || [ "$OSTYPE" == "msys" ]; then
        appName="AlabasterApp.exe"
    fi

    local appDirectory="$(find "$build_and_generator_folder" -type f -name "$appName" | xargs dirname)"

    echo "$appDirectory"
    pushd "$appDirectory" || exit
    pwd
    "./$appName"
    popd || exit
}

run_tests
if [ "$should_run" = "ON" ]; then
    run_app
fi
