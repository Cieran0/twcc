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
        ["add", [10, 22], 32],
    ],
    "add_three": [
        ["add_three", [1, 2, 1], 4],
        ["add_three", [6, 7, 8], (6+7+8)],
        ["add_three", [8, 9, 10], (8+9+10)],
        ["add_three", [10, 22, 0], 32],
    ],
    "mul": [
        ["mul", [1,2], 2],
        ["mul", [2,2], 4],
        ["mul", [6,2], 12],
        ["mul", [8,8], 64],
        ["mul", [100,50], 100*50],
    ],
    "sub": [
        ["sub", [1,2], -1],
        ["sub", [2,2], 0],
        ["sub", [6,2], 4],
        ["sub", [8,8], 0],
        ["sub", [100,50], 50],
    ],
    "div": [
        ["div", [1,2], 0],
        ["div", [2,2], 1],
        ["div", [6,2], 3],
        ["div", [8,8], 1],
        ["div", [100,50], 2],
    ],
    "multiple_functions": [
        ["add", [1, 2], 3],
        ["add", [6, 7], 13],
        ["add", [8, 9], 17],
        ["add", [10, 22], 32],

        ["sub", [1,2], -1],
        ["sub", [2,2], 0],
        ["sub", [6,2], 4],
        ["sub", [8,8], 0],
        ["sub", [100,50], 50],
    ],
    "call_function": [
        ["add_and_double", [1,2], 6],
        ["add_and_double", [5,9], (5+9)*2],
        ["add_and_double", [10,62], 72*2],
        ["add_and_double", [2435,564552], (2435+564552)*2],
        ["add_and_double", [1213,21232], (1213+21232)*2],
    ],
    "numbers": [
        ["double_number", [1], 2]
    ],
    "if": [
        ["say_hey", [0], 0],
        ["say_hey", [1],  1]
    ],
    "local_variables": [
        ["main", [], 200]
    ],
    "complex": [
        ["main", [], 228]
    ],
    "reasignment": [
        ["six", [], 6]
    ],
    "void" : [
        ["main", [], 0]
    ],
    "invalid_void": [
        False
    ],
    "operator_precedence": [
        ["main", [], 7]
    ],
    "else": [
        ["is_five", [5], 1],
        ["is_five", [1], 0],
        ["is_five", [2], 0],
        ["is_five", [3], 0],
        ["is_five", [4], 0],
    ]
    
}


def test_program(program_name):
    source_file = f"examples/{program_name}.c"
    assembly_file = f"testing/{program_name}.s"
    shared_file = f"testing/{program_name}.so"

    result = subprocess.run(
        [
            "./twcc",
            source_file,
            "-o",
            assembly_file
        ],
        stdout=subprocess.DEVNULL
    )

    if result.returncode != 0:
        if len(test_cases[program_name]) == 1 and test_cases[program_name][0] == False:
            print(f"PASS: {program_name} didn't compile!")
        else:
            print("Compilation failed")
        return


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


    try:
        lib = CDLL(os.path.abspath(shared_file))
    except OSError as e:
        print(f"Failed to load shared library: {e}")
        return


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