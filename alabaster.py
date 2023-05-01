from pathlib import Path
import os
from argparse import Action, ArgumentParser, Namespace
import subprocess
import enum
import sys


class BuildMode(enum.Enum):
    Debug = "Debug"
    RelWithDebInfo = "RelWithDebInfo"
    Release = "Release"
    MinSizeRel = "MinSizeRel"


class Generator(enum.Enum):
    Ninja = "Ninja"
    VisualStudio2022 = "VS"


class EnumAction(Action):
    """
    Argparse action for handling Enums
    """

    def __init__(self, **kwargs):
        # Pop off the type value
        enum_type = kwargs.pop("type", None)

        # Ensure an Enum subclass is provided
        if enum_type is None:
            raise ValueError(
                "type must be assigned an Enum when using EnumAction")
        if not issubclass(enum_type, enum.Enum):
            raise TypeError("type must be an Enum when using EnumAction")

        # Generate choices from the Enum
        kwargs.setdefault("choices", tuple(e.value for e in enum_type))

        super(EnumAction, self).__init__(**kwargs)

        self._enum = enum_type

    def __call__(self, parser, namespace, values, option_string=None):
        # Convert value back into an Enum
        value = self._enum(values)
        setattr(namespace, self.dest, value)


def remove_folder_in(base_folder: str, folder: str):
    print("remove_folder_in")


def generate_cmake(generator: Generator, build_folder: str, build_mode: BuildMode, build_tests: bool):
    generator_string = str(
        generator.value) if generator == Generator.Ninja else "Visual Studio 17 2022"

    compile_definitions = [
        f"-G {generator_string}",
        "-D GLFW_INSTALL=OFF",
        "-D GLFW_BUILD_DOCS=OFF",
        "-D GLFW_BUILD_TESTS=OFF",
        "-D GLFW_BUILD_EXAMPLES=OFF",
        f'-D CMAKE_BUILD_TYPE={build_mode.value}',
        f'-D ALABASTER_BUILD_TESTING={build_tests}',
        f'-D BUILD_TESTING={build_tests}',
        "-D ENABLE_HLSL=ON",
        "-D ENTT_BUILD_TESTING=OFF",
        "-D INSTALL_GTEST=OFF",
        "-D Random_BuildTests=OFF",
        "-D SPIRV_SKIP_EXECUTABLES=ON",
        "-D BUILD_SHARED_LIBS=OFF",
        "-D SPDLOG_BUILD_PIC=ON",
        "-D SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=OFF",
        "-D SPIRV_CROSS_SHARED=OFF",
        "-D SPIRV_CROSS_STATIC=ON",
        "-D SPIRV_CROSS_CLI=OFF",
        "-D SPIRV_CROSS_ENABLE_TESTS=OFF",
        "-D SPIRV_CROSS_ENABLE_GLSL=ON",
        "-D SPIRV_CROSS_ENABLE_HLSL=OFF",
        "-D SPIRV_CROSS_ENABLE_MSL=OFF",
        "-D SPIRV_CROSS_ENABLE_CPP=OFF",
        "-D SPIRV_CROSS_ENABLE_REFLECT=OFF",
        "-D SPIRV_CROSS_ENABLE_C_API=OFF",
        "-D SPIRV_CROSS_ENABLE_UTIL=OFF",
        "-D SPIRV_CROSS_SKIP_INSTALL=ON",
        "-D SPIRV_SKIP_TESTS=ON",
        "-D SHADERC_SKIP_INSTALL=ON",
        "-D ENABLE_CTEST=OFF",
        "-D ENABLE_GLSLANG_BINARIES=OFF",
        "-D ENABLE_SPVREMAPPER=OFF",
        "-D SHADERC_SKIP_TESTS=ON",
        "-D SHADERC_SKIP_EXAMPLES=ON",
        "-D SHADERC_SKIP_COPYRIGHT_CHECK=ON",
        "-D SHADERC_ENABLE_WERROR_COMPILE=OFF",
        "-D SKIP_SPIRV_TOOLS_INSTALL=ON",
        "-D SPIRV_BUILD_FUZZER=OFF",
        "-D SPIRV_BUILD_LIBFUZZER_TARGETS=OFF",
        "-D SPIRV_WERROR=OFF",
        "-D SPIRV_WARN_EVERYTHING=OFF",
        "-D SPIRV_COLOR_TERMINAL=OFF",
        "-D SPIRV_TOOLS_LIBRARY_TYPE=STATIC",
        "-D SPDLOG_FMT_EXTERNAL=ON",
        "-D UUID_USING_CXX20_SPAN=ON"]

    builds_args = [f"-B {build_folder}",
                   f"-S {os.path.dirname(os.path.realpath(__file__))}"]

    cmake_args = ['cmake'] + builds_args + compile_definitions

    if sys.platform.startswith('win'):
        shell = True
    else:
        shell = False

    out = subprocess.call(args=cmake_args, shell=shell)
    if out != 0:
        print("Could not configure")
        exit(-1)


def build_cmake(build_folder: str, target: str):
    if sys.platform.startswith('win'):
        shell = True
    else:
        shell = False

    cmake_args = ['cmake', '--build', build_folder,
                  '--target', target, '--parallel', "12"]

    out = subprocess.call(args=cmake_args, shell=shell)
    if out != 0:
        print("Could not build")
        exit(-1)


def run_tests(build_folder: str):
    targets = ['SceneTests', 'CoreTests',
               'ScriptingTests', 'AssetManagerTests']
    for target in targets:
        build_cmake(build_folder, target)

    ctest_args = ['ctest', '-j10', '-C', 'Debug',
                  '--test-dir', build_folder, '--output-on-failure']

    if sys.platform.startswith('win'):
        shell = True
    else:
        shell = False
    out = subprocess.call(args=ctest_args, shell=shell)
    if out != 0:
        print("Tests failed")
        exit(-1)


def run_app():
    print("Running app!")


def main(args: Namespace):
    generator: BuildMode = args.generator
    build_folder = f"build-{args.mode.value}-{generator.value.replace(' ', '')}"

    if args.clean:
        remove_folder_in(build_folder, 'app')
        remove_folder_in(build_folder, 'libs')

    build_folder_exists = Path(build_folder).exists()
    if not build_folder_exists or args.force_configure:
        generate_cmake(generator, build_folder, args.mode, args.build_tests)

    build_cmake(build_folder, args.target)

    if args.build_tests and args.mode == BuildMode.Debug:
        run_tests(build_folder)

    if args.run:
        run_app()


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('-t', '--target', default="Alabaster")
    parser.add_argument('-r', "--run", action='store_true')
    parser.add_argument('-b', '--build-tests', action='store_true')
    parser.add_argument('-f', '--force-configure', action='store_true')
    parser.add_argument('-c', '--clean', action='store_true')
    parser.add_argument('-g', '--generator', type=Generator,
                        action=EnumAction, default=Generator.Ninja)
    parser.add_argument('-m', '--mode', type=BuildMode,
                        action=EnumAction, default=BuildMode.Debug)

    main(parser.parse_args())
