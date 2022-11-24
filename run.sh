#!/usr/bin/env bash

set -e

current_dir="."
build_folder="build"

should_run=${1:OFF}
build_testing=${2:OFF}
clean_first=${3:OFF}
build_type=${4:Debug}

if [ "$clean_first" = "ON" ]; then
	rm -rf "$build_folder/app"
	echo "Cleaned Alabaster::app folder"
	rm -rf "$build_folder/libs"
	echo "Cleaned Alabaster::libs folder"
fi

cmake -B "$build_folder" \
	-G Ninja \
	-D GLFW_INSTALL=OFF \
	-D GLFW_BUILD_DOCS=OFF \
	-D GLFW_BUILD_TESTS=OFF \
	-D GLFW_BUILD_EXAMPLES=OFF \
	-D CMAKE_BUILD_TYPE="$build_type" \
	-D BUILD_TESTING="$build_testing" \
	-D ENABLE_HLSL=ON \
	-D ENABLE_CTEST=OFF \
	-D Random_BuildTests=OFF \
	-D BUILD_SHARED_LIBS=OFF \
	-D SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=OFF \
	-D SPIRV_CROSS_SHARED=OFF \
	-D SPIRV_CROSS_STATIC=ON \
	-D SPIRV_CROSS_CLI=OFF \
	-D SPIRV_CROSS_ENABLE_TESTS=OFF \
	-D SPIRV_CROSS_ENABLE_GLSL=ON \
	-D SPIRV_CROSS_ENABLE_HLSL=OFF \
	-D SPIRV_CROSS_ENABLE_MSL=ON \
	-D SPIRV_CROSS_ENABLE_CPP=OFF \
	-D SPIRV_CROSS_ENABLE_REFLECT=OFF \
	-D SPIRV_CROSS_ENABLE_C_API=OFF \
	-D SPIRV_CROSS_ENABLE_UTIL=OFF \
	-D SPIRV_CROSS_SKIP_INSTALL=ON \
	-S "$current_dir"

cmake --build "$build_folder"

rm "$current_dir/compile_commands.json"
ln -s "build/compile_commands.json" "$current_dir"

run_tests() {
	ctest -j10 --test-dir "build"
}

run_app() {
	pushd "$build_folder/app" || exit
	"./AlabasterApp" "$@"
	popd || exit
}

run_tests
shift 4
if [ "$should_run" = "ON" ]; then
	run_app "$@"
fi
