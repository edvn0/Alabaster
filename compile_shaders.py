#! /usr/bin/env python3

import sys
from typing import List
from pathlib import Path
from subprocess import check_call, CalledProcessError

def main(argc: int, argv: List[str]):
    if (argc != 1):
        raise ValueError("A directory to all shaders must be given (may contain subfolders)")

    shader_io_directory = argv[0]

    all_files = [file for file in Path(shader_io_directory).glob(f"**/*") if file.is_file()]
    all_uncompiled_files = [file for file in all_files if file.suffix != ".spv" and file.suffix in (".vert", ".frag")]
    
    names = [f.name for f in all_uncompiled_files]
    if (len(names) != len(set(names))):
        print("There are duplicate names of shaders. Fix prior to compilation.")
        all_names = list(map(lambda x: x.as_posix(), all_uncompiled_files))
        print("Found duplicates:")
        print("\n".join(all_names))
        print("Invalid output, i.e., the set of names:")
        print(set(names))
        exit(1)

    for uncompiled_file in all_uncompiled_files:
        try:
            call = f"glslc {uncompiled_file} -o {shader_io_directory}/{uncompiled_file.name}.spv"
            check_call(call.split(" "))
        except CalledProcessError as e:
            print(f"Could not compile shader {uncompiled_file.name}. Message: {e.output}")


if __name__ == "__main__":
    sys.argv.pop(0) # drop the filename arg
    main(argc=len(sys.argv), argv=sys.argv)
