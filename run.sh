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

while getopts g:r:t:c:b:h flag
do
    case "${flag}" in
	g) generator=${OPTARG};;
	r) should_run=${OPTARG};;
        t) build_testing=${OPTARG};;
        c) clean_first=${OPTARG};;
        b) build_type=${OPTARG};;
        h) alabaster_help; exit 2;;
        *) alabaster_help; exit 2;;
    esac
done

if [ "$clean_first" = "ON" ]; then
	rm -rf "$build_folder/app"
	echo "Cleaned Alabaster::app folder"
	rm -rf "$build_folder/libs"
	echo "Cleaned Alabaster::libs folder"
fi

if [ "$generator" = "VS" ]; then
	export CMAKE_GENERATOR="Visual Studio 17 2022"
elif [ "$generator" = "Ninja" ]; then
	export CMAKE_GENERATOR="Ninja"
else
	# This may just straight up fail
	export CMAKE_GENERATOR="$generator"
fi;

cmake -B "$build_folder" \
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

if [ "$generator" = "Ninja" ]; then
	rm "$current_dir/compile_commands.json"
	ln -s "build/compile_commands.json" "$current_dir"
fi

run_tests() {
  if [ "$build_type" == "Debug" ] && [ "$build_testing" == "ON" ]
	then
    ctest -j10 --test-dir "build"
  fi;
}

run_app() {
	local appName="AlabasterApp";
	if [ "$generator" = "VS" ]; then
		appName="AlabasterApp.exe"
	fi;

	local appDirectory="$(find ./build -type f -name "$appName" | xargs dirname)"
	pushd "$appDirectory" || exit
	"./$appName"
	popd || exit
}

run_tests
if [ "$should_run" = "ON" ]; then
	run_app
fi
