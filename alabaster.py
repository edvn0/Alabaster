from pathlib import Path
import os
from argparse import Action, ArgumentParser, Namespace
import shutil
import subprocess
import enum
import sys

is_windows = sys.platform.startswith("win")
current_source_dir = os.path.dirname(os.path.realpath(__file__))


class Target(enum.Enum):
    AllBuild = "ALL_BUILD"
    AlabasterApp = "AlabasterApp"
    PythonLibrary = "alabaster_entity"
    SceneTests = "SceneTests"
    CoreTests = "CoreTests"
    ScriptingTests = "ScriptingTests"
    AssetManagerTests = "AssetManagerTests"


class BuildMode(enum.Enum):
    Debug = "Debug"
    RelWithDebInfo = "RelWithDebInfo"
    Release = "Release"
    MinSizeRel = "MinSizeRel"


class Generator(enum.Enum):
    Ninja = "Ninja"
    VS = "VisualStudio"


class EnumAction(Action):
    """
    Argparse action for handling Enums
    """

    def __init__(self, **kwargs):
        # Pop off the type value
        enum_type = kwargs.pop("type", None)

        # Ensure an Enum subclass is provided
        if enum_type is None:
            raise ValueError("type must be assigned an Enum when using EnumAction")
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


def __remove(path: str | Path):
    """param <path> could either be relative or absolute."""

    if not os.path.exists(path):
        return -1

    if os.path.isfile(path) or os.path.islink(path):
        os.remove(path)  # remove the file
        return 0
    elif os.path.isdir(path):
        shutil.rmtree(path)  # remove dir and all contains
        return 0
    else:
        return -1


def remove_folder_in(base_folder: str, folder: str):
    out = subprocess.run(args=["cmake", "--build", base_folder, "--target", "clean"])
    if out != 0:
        print("Could not remove folders")


def make_symlink(file_from: str, file_to: str):
    if Path(file_to).resolve().exists():
        __remove(Path(file_to).resolve())

    p = Path(file_from)
    os.symlink(p.absolute(), Path(file_to).resolve())
    return 0


def generate_cmake(
    generator: Generator, compiler: str, build_folder: str, build_mode: BuildMode, build_tests: bool
):
    generator_string: str = (
        generator.value if generator != Generator.VS else "Visual Studio 17 2022"
    )

    extra_compile_definitions = []
    if "CMAKE_CXX_COMPILER" in os.environ and "CMAKE_C_COMPILER" in os.environ:
        extra_compile_definitions = [
            f"-D CMAKE_CXX_COMPILER={os.environ['CMAKE_CXX_COMPILER']}",
            f"-D CMAKE_C_COMPILER={os.environ['CMAKE_C_COMPILER']}",
        ]

    compile_definitions = [
        f"-G {generator_string}",
        "-D GLFW_INSTALL=OFF",
        "-D GLFW_BUILD_DOCS=OFF",
        "-D GLFW_BUILD_TESTS=OFF",
        "-D GLFW_BUILD_EXAMPLES=OFF",
        f"-D CMAKE_BUILD_TYPE={build_mode.value}",
        f"-D ALABASTER_BUILD_TESTING={'ON' if build_tests else 'OFF'}",
        "-D ALABASTER_IS_BUILD_TOOL=OFF",
        "-D ALABASTER_SHOULD_FORMAT=OFF",
        f"-D BUILD_TESTING={'ON' if build_tests else 'OFF'}",
        "-D ENTT_BUILD_TESTING=OFF",
        "-D INSTALL_GTEST=OFF",
        "-D Random_BuildTests=OFF",
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
        "-D SPDLOG_FMT_EXTERNAL=ON",
        "-D UUID_USING_CXX20_SPAN=ON",
    ]

    compile_shaderc_args = [
        "-D SHADERC_SKIP_TESTS=ON",
        "-D SHADERC_SKIP_EXAMPLES=ON",
        "-D SHADERC_SKIP_COPYRIGHT_CHECK=ON", 
        "-D SPIRV_SKIP_EXECUTABLES=ON",
        "-D BUILD_EXTERNAL=OFF",
        "-D SKIP_GLSLANG_INSTALL=ON",
        "-D ENABLE_GLSLANG_BINARIES=OFF",
        "-D ENABLE_CTEST=OFF",
    ]

    builds_args = [f"-B {build_folder}", f"-S {current_source_dir}"]

    cmake_args = (
        ["cmake"] + builds_args + extra_compile_definitions + compile_definitions
    )

    if compiler.find("clang") != -1 or compiler.find("gcc") != -1:
        cmake_args += compile_shaderc_args

    out = subprocess.run(args=cmake_args, shell=is_windows)
    if out.returncode != 0:
        print("Could not configure")
        exit(out.returncode)


def build_cmake(build_folder: str, target: Target, parallel_jobs: int):
    cmake_args = [
        "cmake",
        "--build",
        build_folder,
        "--target",
        target.value,
        "--parallel",
        f"{parallel_jobs}",
    ]

    out = subprocess.run(args=cmake_args, shell=is_windows)
    if out.returncode != 0:
        print("Could not build")
        exit(out.returncode)


def run_tests(build_folder: str, parallel_jobs: int):
    targets: list[Target] = [
        Target.SceneTests,
        Target.CoreTests,
        Target.ScriptingTests,
        Target.AssetManagerTests,
    ]
    for target in targets:
        build_cmake(build_folder, target, parallel_jobs)

    ctest_args = [
        "ctest",
        "-j10",
        "-C",
        "Debug",
        "--test-dir",
        build_folder,
        "--output-on-failure",
    ]

    out = subprocess.run(args=ctest_args, shell=is_windows)
    if out.returncode != 0:
        print("Tests failed")
        exit(out.returncode)


def run_app(
    build_folder: str, build_mode: BuildMode, has_build_type_inside_build_folder: bool
):
    executable_name = "AlabasterApp"
    executable_location = (
        f"{current_source_dir}/{build_folder}/app/{build_mode.value}"
        if has_build_type_inside_build_folder
        else f"{current_source_dir}/{build_folder}/app"
    )

    args = [Path(executable_location + f"/{executable_name}").resolve()]

    out = subprocess.run(
        args=args, shell=is_windows, cwd=Path(executable_location).resolve()
    )
    if out.returncode != 0:
        print("Fault")
        exit(out.returncode)


def main(args: Namespace):
    generator: Generator = args.generator
    build_mode: BuildMode = args.mode
    target: Target = args.target
    compiler = (
        os.environ["CXX"]
        if "CXX" in os.environ
        else os.environ["CMAKE_CXX_COMPILER"]
        if "CMAKE_CXX_COMPILER" in os.environ
        else "Unknown"
    )
    build_folder = f"build-{build_mode.value}-{generator.value.replace(' ', '')}-{compiler.replace(' ', '')}"

    if args.clean:
        remove_folder_in(build_folder, "app")
        remove_folder_in(build_folder, "libs")

    build_folder_exists = Path(build_folder).exists()
    if not build_folder_exists or args.force_configure:
        generate_cmake(generator, compiler, build_folder, build_mode, args.build_tests)

    build_cmake(build_folder, target, args.parallel)
    should_run_tests = (build_mode == BuildMode.Debug or build_mode ==
                             BuildMode.RelWithDebInfo)
    if args.build_tests and should_run_tests:
        run_tests(build_folder, args.parallel)

    has_build_type_inside_build_folder = any(
        [
            Path(f"{build_folder}/app/{x.name}").exists()
            or Path(f"{build_folder}/libs/Core/{x.name}").exists()
            or Path(f"{build_folder}/libs/AssetManager/{x.name}").exists()
            for x in BuildMode
        ]
    )
    if args.run and args.target == Target.AlabasterApp:
        run_app(build_folder, build_mode, has_build_type_inside_build_folder)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument(
        "-t",
        "--target",
        help="Which target should we build?",
        default=Target.AlabasterApp,
        type=Target,
        action=EnumAction,
    )
    parser.add_argument(
        "-r", "--run", help="Run on completed build", action="store_true"
    )
    parser.add_argument("-b", "--build-tests", help="Build tests", action="store_true")
    parser.add_argument(
        "-f", "--force-configure", help="Reconfigure CMake", action="store_true"
    )
    parser.add_argument(
        "-c", "--clean", help="Clean built targets", action="store_true"
    )
    parser.add_argument(
        "-g",
        "--generator",
        help="Which CMake generator should we use?",
        type=Generator,
        action=EnumAction,
        default=Generator.Ninja,
    )
    parser.add_argument(
        "-m",
        "--mode",
        help="Which build type should we use?",
        type=BuildMode,
        action=EnumAction,
        default=BuildMode.Debug,
    )
    parser.add_argument("-j", "--parallel", help="Parallel build jobs", type=int, choices=range(1,12), default=6)

    main(parser.parse_args())
