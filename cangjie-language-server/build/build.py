# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#

"""cangjie language server build entry"""

import argparse
import os
import platform
import shutil
import stat
import subprocess
import sys
from subprocess import PIPE
from pathlib import Path
from enum import Enum
import concurrent.futures
import re
import json

HOME_DIR = os.path.dirname(Path(__file__).resolve().parent)
BUILD_DIR = os.path.join(HOME_DIR, "build-lsp")
THIRDPARTY_DIR = os.path.join(HOME_DIR, "third_party")
LIB_DIR = os.path.join(HOME_DIR, "src", "lib")
CANGJIE_LIB_DIR = os.path.join(LIB_DIR, "cangjie", "lib", "cjnative")
OUTPUT_DIR = os.path.join(HOME_DIR, "output", "bin")

BUILD_TYPE_MAP = {
    "release": "Release",
    "debug": "Debug",
    "relwithdebinfo": "RelWithDebInfo"
}

class TARGET_SYSTEM_TYPE(Enum):
    NATIVE = "native"
    WINDOWS_X86_64 = "windows-x86_64"
TARGET_SYSTEM_MAP = {
    "native": TARGET_SYSTEM_TYPE.NATIVE,
    "windows-x86_64": TARGET_SYSTEM_TYPE.WINDOWS_X86_64
}
TARGET_SYSTEM = TARGET_SYSTEM_TYPE.NATIVE

IS_WINDOWS = platform.system() == "Windows"
IS_MACOS = platform.system() == "Darwin"
IS_LINUX = platform.system() == "Linux"
JSON_GIT = "https://gitcode.com/openharmony/third_party_json.git"
FLATBUFFER_GIT = "https://gitcode.com/openharmony/third_party_flatbuffers.git"
SQLITE_GIT = "https://gitcode.com/openharmony/third_party_sqlite.git"

def resolve_path(path):
    if os.path.isabs(path):
        return path
    return os.path.abspath(path)
def download_json():
    cmd = ["git", "clone", "-b", "OpenHarmony-v6.0-Release", "--depth=1", JSON_GIT, "json-v3.11.3"]
    subprocess.run(cmd, cwd=THIRDPARTY_DIR, check=True)


def download_flatbuffers(args):
    cmd = ["git", "clone", "-b", "OpenHarmony-v6.0-Release", "--depth=1", FLATBUFFER_GIT, "flatbuffers"]
    subprocess.run(cmd, cwd=THIRDPARTY_DIR, check=True)


def generate_flat_header():
    index_file = os.path.join(HOME_DIR, "generate", "index.fbs")
    flatbuffers_dir = os.path.join(THIRDPARTY_DIR, "flatbuffers")
    new_index_path = os.path.join(flatbuffers_dir, "include", "index.fbs")
    shutil.copy(index_file, new_index_path)

    flatbuffers_build_dir = os.path.join(flatbuffers_dir, "build")
    if not os.path.exists(flatbuffers_build_dir):
        os.makedirs(flatbuffers_build_dir)
    compile_cmd = ["cmake", flatbuffers_dir, "-G", get_generator(), "-DFLATBUFFERS_BUILD_TESTS=OFF"]
    subprocess.run(compile_cmd, cwd=flatbuffers_build_dir, check=True)
    build_cmd = ["cmake", "--build", flatbuffers_build_dir, "-j8"]
    subprocess.run(build_cmd, cwd=flatbuffers_build_dir, check=True)
    flatc_binary = "flatc"
    if IS_WINDOWS:
        flatc_binary = "flatc.exe"
    flac_cmd = os.path.join(flatbuffers_build_dir, flatc_binary)
    generate_cmd = [flac_cmd, "--cpp", index_file]
    subprocess.run(generate_cmd, cwd=flatbuffers_build_dir, check=True)
    header_path = os.path.join(flatbuffers_build_dir, "index_generated.h")
    new_header_path = os.path.join(flatbuffers_dir, "include", "index_generated.h")
    shutil.copy(header_path, new_header_path)

def download_sqlite(args):
    cmd = ["git", "clone", "-b", "OpenHarmony-v6.0-Release", "--depth=1", SQLITE_GIT, "sqlite3"]
    subprocess.run(cmd, cwd=THIRDPARTY_DIR, check=True)

def build_sqlite_amalgamation():
    sqlite_dir = os.path.join(THIRDPARTY_DIR, "sqlite3")
    amalgamation_dir = os.path.join(sqlite_dir, "amalgamation")
    if not os.path.exists(amalgamation_dir):
        os.makedirs(amalgamation_dir)
    shell_c_file = os.path.join(sqlite_dir, "src", "shell.c")
    sqlite3_c_file = os.path.join(sqlite_dir, "src", "sqlite3.c")
    sqlite3_h_file = os.path.join(sqlite_dir, "include", "sqlite3.h")
    sqlite3ext_h_file = os.path.join(sqlite_dir, "include", "sqlite3ext.h")
    new_shell_c_file = os.path.join(amalgamation_dir, "shell.c")
    nwe_sqlite3_c_file = os.path.join(amalgamation_dir, "sqlite3.c")
    nwe_sqlite3_h_file = os.path.join(amalgamation_dir, "sqlite3.h")
    nwe_sqlite3ext_h_file = os.path.join(amalgamation_dir, "sqlite3ext.h")
    shutil.copy(shell_c_file, new_shell_c_file)
    shutil.copy(sqlite3_c_file, nwe_sqlite3_c_file)
    shutil.copy(sqlite3_h_file, nwe_sqlite3_h_file)
    shutil.copy(sqlite3ext_h_file, nwe_sqlite3ext_h_file)

def prepare_cangjie(args):
    cangjie_sdk_path = resolve_path(os.getenv("CANGJIE_HOME"))
    if not os.path.exists(LIB_DIR):
        os.makedirs(LIB_DIR)
    if not os.path.exists(CANGJIE_LIB_DIR):
        os.makedirs(CANGJIE_LIB_DIR)
    if os.path.exists(cangjie_sdk_path):
        # copy header files
        cangjie_header_dir = os.path.join(cangjie_sdk_path, "include")
        new_header_dir = os.path.join(LIB_DIR, "cangjie", "include")
        shutil.copytree(cangjie_header_dir, new_header_dir, dirs_exist_ok=True)
        # copy libraries
        tools_lib_dir = os.path.join(cangjie_sdk_path, "tools", "lib")
        tools_bin_dir = os.path.join(cangjie_sdk_path, "tools", "bin")
        file_extension = ""
        if IS_WINDOWS or TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
            file_extension = "dll"
        if IS_LINUX and not TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
            file_extension = "so"
        if IS_MACOS:
            file_extension = "dylib"
        cangjie_lib_path = os.path.join(tools_lib_dir, "libcangjie-lsp." + file_extension)
        if IS_WINDOWS or TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
            cangjie_lib_path = os.path.join(tools_bin_dir, "libcangjie-lsp." + file_extension)
        new_lib_path = os.path.join(CANGJIE_LIB_DIR, "libcangjie-lsp." + file_extension)
        shutil.copy(cangjie_lib_path, new_lib_path)
        if IS_WINDOWS or TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
            cangjie_lib_path = os.path.join(tools_lib_dir, "libcangjie-lsp.dll.a")
            new_lib_path = os.path.join(CANGJIE_LIB_DIR, "libcangjie-lsp.dll.a")
            shutil.copy(cangjie_lib_path, new_lib_path)

def prepare_build(args):
    prepare_cangjie(args)
    if not os.path.exists(THIRDPARTY_DIR):
        os.makedirs(THIRDPARTY_DIR)
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "json-v3.11.3")):
        download_json()
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "flatbuffers")):
        download_flatbuffers(args)
        generate_flat_header()
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "sqlite3")):
        download_sqlite(args)
        build_sqlite_amalgamation()

def get_generator():
    generator = "Unix Makefiles"
    if IS_WINDOWS:
        generator = "MinGW Makefiles"
    return generator

def get_compiler_type():
    compiler_type = "Ninja"
    if IS_WINDOWS:
        compiler_type = "MINGW"
    return compiler_type

def get_build_commands(args):
    result = [
        "cmake",
        "--build",
        BUILD_DIR
    ]
    if args.jobs > 0:
        result.extend(["-j", str(args.jobs)])
    return result

def generate_cmake_commands(args):
    # cmake .. -G "MinGW Makefiles" -DCOMPILER_TYPE=MINGW -DCMAKE_BUILD_TYPE=Release
    result = [
        "cmake", HOME_DIR,
        "-G", get_generator(),
        "-DCOMPILER_TYPE=" + get_compiler_type(),
        "-DCMAKE_BUILD_TYPE=" + BUILD_TYPE_MAP[args.build_type]
    ]
    if TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
        result.extend([
            "-DCROSS_WINDOWS=ON",
            "-DCMAKE_SYSTEM_NAME=Windows",
            "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
            "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++"
        ])
    if args.test:
        result.extend([
            "-DENABLE_TEST=ON",
            "-DENABLE_COVERAGE=ON",
            "-DNO_EXCEPTIONS=ON",
            "-DCMAKE_CXX_FLAGS='-fno-exceptions'"
        ])
    return result

def build(args):
    print("start build")
    if os.getenv("CANGJIE_HOME") is None:
        print("build failed, 'CANGJIE_HOME' environment variable not found, please config it first.")
        return
    if args.target_system is not None:
        global TARGET_SYSTEM
        TARGET_SYSTEM = TARGET_SYSTEM_MAP[args.target_system]
    prepare_build(args)
    cmake_command = generate_cmake_commands(args)
    build_command = get_build_commands(args)
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)
        subprocess.run(cmake_command, cwd=BUILD_DIR, check=True)
    print(build_command)
    subprocess.run(build_command, cwd=BUILD_DIR, check=True)
    print("end build")

def redo_with_write(redo_func, path, err):
    # Is the error an access error?
    if not os.access(path, os.W_OK):
        os.chmod(path, stat.S_IWUSR)
        redo_func(path)
    else:
        raise

def delete_folder(folder_path):
    try:
        shutil.rmtree(folder_path, onerror=redo_with_write)
    except Exception as e:
        print(f"delete {folder_path} failed: {str(e)}")

def clean(args):
    """
    clean target
    output build-lsp
    third_party/json-v3.11.3
    third_party/flatbuffers
    third_party/sqlite3
    src/lib
    """
    print("start clean")
    delete_folders = ["output", "build-lsp"]
    for folder in delete_folders:
        folder = os.path.join(HOME_DIR, folder)
        if os.path.exists(folder):
            delete_folder(os.path.join(HOME_DIR, folder))
    if os.path.exists(THIRDPARTY_DIR):
        delete_folder(THIRDPARTY_DIR)
    third_party_folders = ["json-v3.11.3", "flatbuffers", "sqlite3"]
    for folder in third_party_folders:
        folder = os.path.join(THIRDPARTY_DIR, folder)
        if os.path.exists(folder):
            delete_folder(os.path.join(THIRDPARTY_DIR, folder))
    if os.path.exists(LIB_DIR):
        delete_folder(LIB_DIR)
    print("end clean")

def install(args):
    binary_path = os.path.join(HOME_DIR, "output", "bin")
    if not os.path.exists(binary_path):
        print("install failed, binary not found, please run build first.")
        return
    if args is None or args.install_path is None:
        print(f"no path is specified, the binary is in the '{binary_path}' path.")
        return
    install_path = os.path.abspath(args.install_path)
    if not os.path.exists(install_path):
        print("install failed, target install path does not exist.")
        return
    if not os.path.isdir(install_path):
        print("install failed, target install path is not a folder.")
        return
    shutil.copytree(binary_path, install_path, dirs_exist_ok=True)
    print(f"target path '{install_path}' installed successfully.")

def get_run_test_command(cangjie_sdk_path):
    env_file = "envsetup.sh"
    gtest_file = "gtest_LSPServer_test"
    if IS_WINDOWS:
        env_file = "envsetup.bat"
        gtest_file = "gtest_LSPServer_test.exe"
    env_path = os.path.join(resolve_path(cangjie_sdk_path), env_file)
    test_path = os.path.join(OUTPUT_DIR, gtest_file)
    result = []
    if not IS_WINDOWS:
        result.extend(["bash", "-c", "source " + env_path + " && " + test_path])
    else:
        result.extend([env_path, "&&", test_path])
    return result

def get_test_list(cangjie_sdk_path):
    gtest_file = "gtest_LSPServer_test"
    env_file = "envsetup.sh"
    if IS_WINDOWS:
        gtest_file = "gtest_LSPServer_test.exe"
        env_file = "envsetup.bat"
    env_path = os.path.join(resolve_path(cangjie_sdk_path), env_file)
    test_path = os.path.join(OUTPUT_DIR, gtest_file)    
    
    if not IS_WINDOWS:
        list_command = ["bash", "-c", f"source {env_path} && {test_path} --gtest_list_tests"]
    else:
        list_command = [env_path, "&&", f"{test_path} --gtest_list_tests"]
    
    try:
        result = subprocess.run(list_command, cwd=OUTPUT_DIR, capture_output=True, text=True)
        test_list_output = result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Failed to get test list: {e}")
        exit(1)
    
    parallel_test_suites = []
    serial_test_suites = []
    for line in test_list_output.split('\n'):
        # 测试套名称通常在行首，没有缩进
        if line and not line.startswith(' ') and '.' in line:
            test_suite = line.split('.')[0]
            if '/' in test_suite and test_suite not in parallel_test_suites:
                parallel_test_suites.append(test_suite)
            elif test_suite not in serial_test_suites:
                serial_test_suites.append(test_suite)
    
    print(f"Found {len(parallel_test_suites)} parallel test suites: {parallel_test_suites}")
    print(f"Found {len(serial_test_suites)} serial test suites: {serial_test_suites}")
    return parallel_test_suites, serial_test_suites

def run_single_test_suite(cangjie_sdk_path, test_suite):
    env_file = "envsetup.sh"
    gtest_file = "gtest_LSPServer_test"
    if IS_WINDOWS:
        env_file = "envsetup.bat"
        gtest_file = "gtest_LSPServer_test.exe"
    
    env_path = os.path.join(resolve_path(cangjie_sdk_path), env_file)
    test_path = os.path.join(OUTPUT_DIR, gtest_file)
    test_report_name = f"result_{test_suite}.json"
    if '/' in test_report_name:
        test_report_name = test_report_name.replace('/', '.')

    if not IS_WINDOWS:
        command = ["bash", "-c", f"source {env_path} && {test_path} --gtest_filter={test_suite}* --gtest_output=json:{test_report_name}"]
    else:
        command = [env_path, "&&", f"{test_path} --gtest_filter={test_suite}* --gtest_output=json:{test_report_name}"]
    
    try:
        result = subprocess.run(command, cwd=OUTPUT_DIR, check=True)
        if result.returncode == 0:
            return True, test_suite, result.stdout, result.stderr
        else:
            return False, test_suite, result.stdout, result.stderr
    except Exception as e:
        return False, test_suite, None, str(e)

def test(args):
    print("start run test")
    cangjie_sdk_path = resolve_path(os.getenv("CANGJIE_HOME"))
    if not os.path.exists(cangjie_sdk_path):
        print("set cangjie sdk path not exists")
        return
    if not os.path.exists(OUTPUT_DIR):
        print("no output/bin path")
        return
    
    parallel_test_suites, serial_test_suites = get_test_list(cangjie_sdk_path)
    
    max_workers = args.jobs if hasattr(args, 'jobs') and args.jobs > 0 else min(32, len(parallel_test_suites) + len(serial_test_suites))
    
    failed_suites = []
    
    # Run parallel and serial test suites concurrently
    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as main_executor:
        # Submit parallel test suites task
        parallel_future = main_executor.submit(run_parallel_tests, cangjie_sdk_path, parallel_test_suites, max_workers)
        # Submit serial test suites task
        serial_future = main_executor.submit(run_serial_tests, cangjie_sdk_path, serial_test_suites)
        
        # Collect results from both
        parallel_failed = parallel_future.result()
        serial_failed = serial_future.result()
        
        failed_suites.extend(parallel_failed)
        failed_suites.extend(serial_failed)
    
    if failed_suites:
        print('\n=== Test Summary ===')
        total_suites = len(parallel_test_suites) + len(serial_test_suites)
        print(f'Total suites: {total_suites}, Passed: {total_suites - len(failed_suites)}, Failed: {len(failed_suites)}')
        print('Failed test suites:')
        failed_count = 0
        for suite in failed_suites:
            print(f'  - {suite}')
            test_report_name = f"result_{suite}.json"
            if '/' in test_report_name:
                test_report_name = test_report_name.replace('/', '.')
            result_path = os.path.join(OUTPUT_DIR, test_report_name)
            if not os.path.exists(result_path):
                print(f'result file {test_report_name} not found.')
                continue
            with open(result_path, 'r') as json_name:
                json_to_dict = json.load(json_name)
                if len(json_to_dict['testsuites']) == 0:
                    continue
                failed_count += json_to_dict['failures']
                for testsuite in json_to_dict['testsuites'][0]['testsuite']:
                    if 'failures' in testsuite.keys(): 
                        fail_test = testsuite['classname'] + '.' + testsuite['name']
                        if 'value_param' in testsuite:
                            fail_test = fail_test + ', where GetParam() = ' + testsuite['value_param']
                        print(f'[ FAILED ] {fail_test}')
        print(f'\nTest failed: {failed_count}')
        exit(1)
    else:
        print("All test suites passed")
    
    print("end run test")

def run_parallel_tests(cangjie_sdk_path, parallel_test_suites, max_workers):
    """Run parallel test suites concurrently"""
    failed_suites = []
    if parallel_test_suites:
        print(f"Running {len(parallel_test_suites)} parallel test suites concurrently...")
        with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
            # submit tasks
            future_to_suite = {
                executor.submit(run_single_test_suite, cangjie_sdk_path, suite): suite 
                for suite in parallel_test_suites
            }
            
            # collect results
            for future in concurrent.futures.as_completed(future_to_suite):
                success, suite, stdout, stderr = future.result()
                if not success:
                    failed_suites.append(suite)
    return failed_suites

def run_serial_tests(cangjie_sdk_path, serial_test_suites):
    """Run serial test suites sequentially"""
    failed_suites = []
    if serial_test_suites:
        print(f"Running {len(serial_test_suites)} serial test suites sequentially...")
        for suite in serial_test_suites:
            success, suite, stdout, stderr = run_single_test_suite(cangjie_sdk_path, suite)
            if not success:
                failed_suites.append(suite)
    return failed_suites

def main():
    """build entry"""
    parser = argparse.ArgumentParser(description="build lspserver project")
    subparsers = parser.add_subparsers(help="sub command help")
    parser_build = subparsers.add_parser("build", help=" build cangjie")
    parser_build.add_argument(
        "-t",
        "--build-type",
        choices=["release", "debug", "relwithdebinfo"],
        dest="build_type",
        type=str,
        default="release",
        help="select target build type"
    )
    parser_build.add_argument(
        "-j", "--jobs", dest="jobs", type=int, default=0, help="run N jobs in parallel"
    )
    parser_build.add_argument(
        "--target",
        choices=["native", "windows-x86_64"],
        dest="target_system",
        type=str,
        default="native",
        help="set build target system"
    )
    parser_build.add_argument(
        "--test",
        action="store_true",
        dest="test",
        help="set for test LSP"
    )
    parser_build.set_defaults(func=build)

    parser_clean = subparsers.add_parser("clean", help="clean build")
    parser_clean.set_defaults(func=clean)

    parser_install = subparsers.add_parser("install", help="install binary")
    parser_install.add_argument(
        "--prefix",
        dest="install_path",
        type=str,
        help="set installation path"
    )
    parser_install.set_defaults(func=install)

    parser_test = subparsers.add_parser("test", help="run test case")
    parser_test.set_defaults(func=test)

    args = parser.parse_args()
    args.func(args)

if __name__ == "__main__":
    os.environ["LANG"] = "C.UTF-8"
    main()