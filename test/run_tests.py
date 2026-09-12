#!/usr/bin/env python3

import os
import shutil
import subprocess
from ctypes import *

test_cases = {
    "simpliest": [
        ["add", [1, 2], 3],
        ["add", [6, 7], 13],
        ["add", [8, 9], 17],
        ["add", [10, 22], 32]
    ],
    "mul": [
        ["mul", [1,2], 2],
        ["mul", [2,2], 4],
        ["mul", [6,2], 12],
        ["mul", [8,8], 64],
        ["mul", [100,50], 100*50],
    ]
}


def test_program(program_name):
    source_file = f"examples/{program_name}.c"
    assembly_file = f"testing/{program_name}.s"
    shared_file = f"testing/{program_name}.so"

    print(f"Compiling {source_file}...")

    result = subprocess.run(
        [
            "./twcc",
            source_file,
            assembly_file
        ],
        stdout=subprocess.DEVNULL
    )

    if result.returncode != 0:
        print("Compilation failed")
        return

    print("Compilation succeeded")

    print(f"Building {shared_file}...")

    result = subprocess.run(
        [
            "gcc",
            "-shared",
            "-o",
            shared_file,
            assembly_file
        ],
        stdout=subprocess.DEVNULL
    )

    if result.returncode != 0:
        print("Shared library build failed")
        return

    print("Shared library built")

    try:
        lib = CDLL(os.path.abspath(shared_file))
    except OSError as e:
        print(f"Failed to load shared library: {e}")
        return

    print(f"Loaded {shared_file}")

    for function_name, args, expected in test_cases[program_name]:

        try:
            func = getattr(lib, function_name)
        except AttributeError:
            print(f"FAIL: {function_name} not found")
            continue

        func.argtypes = [c_int] * len(args)
        func.restype = c_int

        try:
            result = func(*args)
        except Exception as e:
            print(f"FAIL: {function_name}{args}: {e}")
            continue

        if result == expected:
            print(f"PASS: {function_name}{args} == {result}")
        else:
            print(
                f"FAIL: {function_name}{args}: "
                f"expected {expected}, got {result}"
            )


if os.path.exists("testing"):
    shutil.rmtree("testing")

os.mkdir("testing")

for program_name in test_cases.keys():
    test_program(program_name)

shutil.rmtree("testing")