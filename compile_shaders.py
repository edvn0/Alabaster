#! /usr/bin/env python3

import dataclasses
import json
import stat
import sys
from dataclasses import dataclass
from argparse import ArgumentParser, Namespace
from subprocess import call, DEVNULL
import datetime
from pathlib import Path
from subprocess import check_call, CalledProcessError
from typing import List

SUFFIXES = (".vert", ".frag", ".comp", ".geom")


class EnhancedJSONEncoder(json.JSONEncoder):
    def default(self, o):
        if dataclasses.is_dataclass(o):
            return dataclasses.asdict(o)
        return super().default(o)


@dataclass
class CacheRecord:
    compiled_name: str
    file_name: str
    timestamp: str


def __shader_file_filter(path: Path) -> bool:
    return path.is_file() and path.suffix != ".spv" and path.suffix in SUFFIXES


def filter_shaders_from_directory(directory: str) -> List[Path]:
    shader_glob = Path(directory).glob("**/*")
    return list(filter(__shader_file_filter, [file for file in shader_glob]))


def verify_no_duplicate_names(uncompiled_files: List[Path]):
    names = [f.name for f in uncompiled_files]
    if len(names) != len(set(names)):
        print("There are duplicate names of shaders. Fix prior to compilation.")
        all_names = list(map(lambda x: x.as_posix(), uncompiled_files))
        print("Found duplicates:")
        print("\n".join(all_names))
        print("Invalid output, i.e., the set of names:")
        print(set(names))
        exit(1)


def find_compiler(compiler: str):
    try:
        call([compiler, '-h'], stdout=DEVNULL)
        return True
    except FileNotFoundError:
        return False


def load_cache(parsed: Namespace) -> List[CacheRecord]:
    cache_name = str(parsed.cache_name)
    if cache_name.endswith(".cache"):
        cache_name = cache_name.split(".cache")[0]

    try:
        with open(f'{parsed.dir}/{cache_name}.cache', 'r') as cache_file:
            cache = json.load(cache_file)
            records = map(lambda x: CacheRecord(compiled_name=x['compiled_name'], file_name=x['file_name'],
                                                timestamp=x['timestamp']), cache)
            return list(records)
    except FileNotFoundError:
        return []


def write_cache(parsed: Namespace, records: List[CacheRecord]) -> None:
    try:
        with open(f'{parsed.dir}/{parsed.cache_name}.cache', 'w') as cache_file:
            cache_file.writelines(json.dumps(records, cls=EnhancedJSONEncoder))
            cache_file.write("\n")
    except FileNotFoundError:
        print("Could not write to cache file.")


def compile_to_spirv(parsed: Namespace, uncompiled_file: Path) -> CacheRecord:
    try:
        compile_call = f"{parsed.compiler} {uncompiled_file} -o {parsed.output_dir}/{uncompiled_file.name}.spv"
        check_call(compile_call.split(" "))
        return CacheRecord(file_name=uncompiled_file.as_posix(),
                           compiled_name=f"{parsed.dir}/{uncompiled_file.name}.spv",
                           timestamp=datetime.datetime.now().isoformat())
    except CalledProcessError as e:
        print(
            f"Could not compile shader {uncompiled_file.name}. Message: {e.output}")


def main(parsed: Namespace):
    all_uncompiled_files = filter_shaders_from_directory(parsed.dir)
    if not find_compiler(parsed.compiler):
        print("Could not find compiler.")
        exit(1)

    cached_records = load_cache(parsed)

    compiled_file_cache: List[CacheRecord] = []
    for uncompiled_file in all_uncompiled_files:
        record = compile_to_spirv(parsed, uncompiled_file)
        compiled_file_cache.append(record)

    write_cache(parsed, compiled_file_cache)


if __name__ == "__main__":
    parser = ArgumentParser(
        description='Finds all shaders in [dir] (recursively), and compiles with [compiler=glslc]. They are output '
                    'into [output-dir], i.e. the root of the recursive search.')
    parser.add_argument("--dir", type=str, metavar='D',
                        help='The directory containing shaders and potentially folders containing shaders, recursively.')
    parser.add_argument("--output-dir", type=str, metavar='O',
                        help="Output directory of shaders. This tool flattens all shaders into this directory.")
    parser.add_argument("--compiler", type=str, metavar='C',
                        help="The path to your choice of shader compiler", default="/usr/bin/glslc")
    parser.add_argument("--cache-name", type=str, metavar='S', help="The filename of cached compiled shaders",
                        default="shaders.cache")
    parser.add_argument("--force-reload", type=bool, default=False, metavar='F',
                        help="Recompile shader regardless of their cache status.")
    main(parser.parse_args())
