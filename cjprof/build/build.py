#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
# This source file is part of the Cangjie Project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

"""Cangjie Profile build entry"""
import argparse
import logging
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys
from enum import Enum
from logging.handlers import TimedRotatingFileHandler
from subprocess import PIPE

THREADS_NUM = multiprocessing.cpu_count() // 2
FAIL_CASE_NUM = 0
PASS_CASE_NUM = 0

BUILD_DIR = os.path.dirname(os.path.abspath(__file__))
HOME_DIR = os.path.dirname(BUILD_DIR)
EXECUTE_DIR = os.path.join(BUILD_DIR, "build", "bin")
THIRDPARTY_DIR = os.path.join(HOME_DIR, "third_party")
CMAKE_BUILD_DIR = os.path.join(BUILD_DIR, "build")
CMAKE_OUTPUT_DIR = os.path.join(HOME_DIR, "dist")
LOG_DIR = os.path.join(BUILD_DIR, "logs")
LOG_FILE = os.path.join(LOG_DIR, "cjprof.log")

IS_WINDOWS = platform.system() == "Windows"
IS_MAC = platform.system() == "Darwin"

def run_jni_script(args):
    jni_script_file = os.path.join(BUILD_DIR, "../bridge/jni/build.py")
    cmd = [sys.executable, jni_script_file]
    cmd.extend(args)
    
    try:
        result = subprocess.run(
            cmd,
            check=True,
            capture_output=True,
            text=True,
            cwd=None
        )
        print(result.stdout)
        
    except subprocess.CalledProcessError as e:
        print(f"return code: {e.returncode}")
        print(f"error: {e.stderr}")
    except FileNotFoundError:
        print("cannot find build.py")

def download_boundscheck():
    cmd = ["git", "clone", "-b", "OpenHarmony-v6.0-Release", "https://gitcode.com/openharmony/third_party_bounds_checking_function.git", "boundscheck"]
    output = subprocess.Popen(cmd, cwd=THIRDPARTY_DIR, stdout=PIPE)
    for line in output.stdout:
        print(line.decode("ascii", "ignore").rstrip())

def download_runtime():
    cmd = ["git", "clone", "-b", "main", "https://gitcode.com/Cangjie/cangjie_runtime.git", "runtime"]
    output = subprocess.Popen(cmd, cwd=THIRDPARTY_DIR, stdout=PIPE)
    for line in output.stdout:
        print(line.decode("ascii", "ignore").rstrip())

def download_json():
    cmd = ["git", "clone", "-b", "OpenHarmony-v6.0-Release", "--depth=1", "https://gitcode.com/openharmony/third_party_json.git", "third_party_json"]
    output = subprocess.Popen(cmd, cwd=THIRDPARTY_DIR, stdout=PIPE)
    for line in output.stdout:
        print(line.decode("ascii", "ignore").rstrip())

def download_httplib():
    cmd = ["git", "clone", "-b", "master", "--depth=1", "https://gitcode.com/GitHub_Trending/cp/cpp-httplib.git", "cpp-httplib"]
    output = subprocess.Popen(cmd, cwd=THIRDPARTY_DIR, stdout=PIPE)
    for line in output.stdout:
        print(line.decode("ascii", "ignore").rstrip())

def prepare_build():
    if not os.path.exists(THIRDPARTY_DIR):
        os.makedirs(THIRDPARTY_DIR)
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "runtime")):
        download_runtime()
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "boundscheck")):
        download_boundscheck()
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "third_party_json")):
        download_json()
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "cpp-httplib")):
        download_httplib()

def setup_compiler():
    if not IS_WINDOWS:
        clang_path = shutil.which("clang")
        clang_pp_path = shutil.which("clang++")
        if not clang_path:
            LOG.error("clang is required to build cangjie compiler")
            sys.exit(1)
        if not clang_pp_path:
            LOG.error("clang++ is required to build cangjie compiler")
            sys.exit(1)

        os.environ["CC"] = clang_path
        os.environ["CXX"] = clang_pp_path
    else:
        c_compiler = shutil.which("gcc")
        cxx_compiler = shutil.which("g++")
        if not c_compiler:
            LOG.error("gcc is required to build cangjie compiler")
            sys.exit(1)
        if not cxx_compiler:
            LOG.error("g++ is required to build cangjie compiler")
            sys.exit(1)
        os.environ["CC"] = c_compiler
        os.environ["CXX"] = cxx_compiler


def log_output(output):
    """log command output"""
    while True:
        line = output.stdout.readline()
        if not line:
            output.communicate()
            returncode = output.returncode
            if returncode != 0:
                LOG.error("build error: %d!\n", returncode)
                sys.exit(1)
            break
        try:
            LOG.info(line.decode("ascii", "ignore").rstrip())
        except UnicodeEncodeError:
            LOG.info(line.decode("utf-8", "ignore").rstrip())


def init_log(name):
    """init log config"""
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)

    log = logging.getLogger(name)
    log.setLevel(logging.DEBUG)
    formatter = logging.Formatter(
        "[%(asctime)s:%(module)s:%(lineno)s:%(levelname)s] %(message)s"
    )
    streamhandler = logging.StreamHandler(sys.stdout)
    streamhandler.setLevel(logging.DEBUG)
    streamhandler.setFormatter(formatter)
    log.addHandler(streamhandler)
    filehandler = TimedRotatingFileHandler(
        LOG_FILE, when="W6", interval=1, backupCount=60
    )
    filehandler.setLevel(logging.DEBUG)
    filehandler.setFormatter(formatter)
    log.addHandler(filehandler)
    return log


def generate_cmake_defs(args):
    """convert args to cmake defs"""

    def bool_to_opt(value):
        return "ON" if value else "OFF"

    build_type = BuildType.debug.value if args.code_coverage else args.build_type.value
    coverage_defs = []
    if args.code_coverage:
        coverage_defs = [
            "-DCMAKE_CXX_FLAGS=-fprofile-arcs -ftest-coverage -DCOVERAGE_BUILD",
            "-DCMAKE_C_FLAGS=-fprofile-arcs -ftest-coverage",
        ]

    # --build-test enables the gtest-based C++ API tests (tests/ subdirectory).
    # It is independent of --code-coverage so the coverage build keeps its
    # original semantics (gcov instrumentation + force Debug only); opt into
    # the test target explicitly when you want it built (e.g. for coverage
    # collection). The tests are NOT run here; run them via ctest later.
    test_defs = []
    if getattr(args, "build_test", False):
        test_defs.append("-DBUILD_TESTING=ON")

    if args.target == "windows-x86_64":
        return [
            "-DCMAKE_BUILD_TYPE=" + build_type,
            "-DCMAKE_SYSTEM_NAME=" + "Windows",
            "-DCMAKE_C_COMPILER=" + "x86_64-w64-mingw32-gcc",
            "-DCMAKE_CXX_COMPILER=" + "x86_64-w64-mingw32-g++",
            "-DSTRIP_LIB_PREFIX=" + "ON",
            "-DUSE_CXX17_FEATURES=" + "ON",
        ] + coverage_defs + test_defs + [arg for arg in args.cmake_args if arg != "--"]

    if IS_MAC:
        return [
            "-DCMAKE_BUILD_TYPE=" + build_type,
            "-DUSE_CXX17_FEATURES=" + "ON",
        ] + coverage_defs + test_defs + [arg for arg in args.cmake_args if arg != "--"]

    if IS_WINDOWS:
        return [
            "-DCMAKE_BUILD_TYPE=" + build_type,
            "-DSTRIP_LIB_PREFIX=" + "ON",
            "-DUSE_CXX17_FEATURES=" + "ON",
        ] + coverage_defs + test_defs + [arg for arg in args.cmake_args if arg != "--"]

    return [
        "-DCMAKE_BUILD_TYPE=" + build_type,
    ] + coverage_defs + test_defs + [arg for arg in args.cmake_args if arg != "--"]


def build(args):
    if args.code_coverage:
        args.build_type = BuildType.debug
    if args.build_type is None:
        LOG.error(
            "please specify the build type. Supported options are: release and debug."
        )
        return

    prepare_build()
    """cjprof build"""
    LOG.info("begin build...\n")

    """build windows executables from Linux"""
    if args.target == "windows-x86_64":
        if not os.path.exists(CMAKE_BUILD_DIR):
            os.makedirs(CMAKE_BUILD_DIR)
        output = subprocess.Popen(
            ["cmake", HOME_DIR] + generate_cmake_defs(args),
            cwd=CMAKE_BUILD_DIR,
            stdout=PIPE,
        )
        log_output(output)
        output = subprocess.Popen(
            ["make", "-j" + str(args.jobs)], cwd=CMAKE_BUILD_DIR, stdout=PIPE
        )
        log_output(output)
        if not args.skip_jni:
            run_jni_script(["build", "-t", str(args.build_type), "--target", str(args.target)])
        else:
            LOG.info("skipping JNI build (--skip-jni)\n")
        LOG.info("end build\n")
        return

    if os.path.exists(CMAKE_BUILD_DIR):
        if IS_WINDOWS:
            output = subprocess.Popen(
                ["mingw32-make", "-j" + str(args.jobs)],
                cwd=CMAKE_BUILD_DIR,
                stdout=PIPE,
            )
        else:
            output = subprocess.Popen(["ninja", "-j" + str(args.jobs)], cwd=CMAKE_BUILD_DIR, stdout=PIPE)
        log_output(output)
    else:
        os.makedirs(CMAKE_BUILD_DIR)
        if IS_WINDOWS:
            generator = "MinGW Makefiles"
        else:
            generator = "Ninja"
        output = subprocess.Popen(
            ["cmake", HOME_DIR, "-G", generator] + generate_cmake_defs(args),
            cwd=CMAKE_BUILD_DIR,
            stdout=PIPE,
        )
        log_output(output)
        if IS_WINDOWS:
            output = subprocess.Popen(
                ["mingw32-make", "-j" + str(args.jobs)],
                cwd=CMAKE_BUILD_DIR,
                stdout=PIPE,
            )
        else:
            output = subprocess.Popen(["ninja", "-j" + str(args.jobs)], cwd=CMAKE_BUILD_DIR, stdout=PIPE)
        log_output(output)
    if getattr(args, "build_test", False):
        LOG.info("building gtest target cjprof_gtest (--build-test)\n")
        make_tool = "mingw32-make" if IS_WINDOWS else "ninja"
        output = subprocess.Popen(
            [make_tool, "-j" + str(args.jobs), "cjprof_gtest"],
            cwd=CMAKE_BUILD_DIR,
            stdout=PIPE,
        )
        log_output(output)
        if output.returncode != 0:
            LOG.fatal("build cjprof_gtest failed")
        else:
            LOG.info("cjprof_gtest built — run via: ctest --test-dir %s\n" % CMAKE_BUILD_DIR)
    if not args.skip_jni:
        run_jni_script(["build", "-t", str(args.build_type)])
    else:
        LOG.info("skipping JNI build (--skip-jni)\n")
    LOG.info("end build\n")


def clean(args):
    """clean build outputs and logs"""
    LOG.info("begin clean...\n")
    output_dirs = [
        "build/build"
    ]
    for file_path in output_dirs:
        abs_file_path = os.path.join(HOME_DIR, file_path)
        if os.path.isdir(abs_file_path):
            shutil.rmtree(abs_file_path, ignore_errors=True)
        if os.path.isfile(abs_file_path):
            os.remove(abs_file_path)
    if not args.skip_jni:
        run_jni_script(["clean"])
    else:
        LOG.info("skipping JNI clean (--skip-jni)\n")
    LOG.info("end clean\n")


def install(args):
    """install targets"""
    LOG.info("begin install targets...")
    install_path = args.install_prefix if args.install_prefix else CMAKE_OUTPUT_DIR
    output = subprocess.Popen(
        ["cmake", "--install", "build", "--prefix", install_path],
        cwd=BUILD_DIR,
        stdout=PIPE,
    )
    log_output(output)
    if output.returncode != 0:
        LOG.fatal("install failed")
    run_jni_script(["install"])

    shutil.copytree(os.path.join(BUILD_DIR, "..", "bridge", "jni", "dist"), os.path.join(install_path, "lib"), dirs_exist_ok=True)
    LOG.info("end install targets...")


class BuildType(Enum):
    """CMAKE_BUILD_TYPE options"""

    debug = "Debug"
    release = "Release"

    def __str__(self):
        return self.name

    def __repr__(self):
        return str(self)

    @staticmethod
    def argparse(s):
        try:
            return BuildType[s]
        except KeyError:
            return s


def main():
    """build entry"""
    parser = argparse.ArgumentParser(description="build cjprof project")
    subparsers = parser.add_subparsers(help="sub command help")
    parser_build = subparsers.add_parser("build", help="build cjprof")
    parser_build.add_argument(
        "-t",
        "--build-type",
        type=BuildType.argparse,
        dest="build_type",
        default=None,
        choices=list(BuildType),
        help="select target build type",
    )
    parser_build.add_argument(
        "--code-coverage",
        action="store_true",
        help="build with gcov instrumentation (-fprofile-arcs -ftest-coverage) and force Debug",
    )
    parser_build.add_argument(
        "--target",
        type=str,
        dest="target",
        default="native",
        choices=["native", "windows-x86_64"],
        help="Specify the target platform (default: native).",
    )
    parser_build.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=THREADS_NUM,
        help="number of parallel jobs to run")
    parser_build.add_argument(
        "--skip-jni",
        action="store_true",
        help="skip JNI/Java bridge build")
    parser_build.add_argument(
        "--build-test",
        action="store_true",
        dest="build_test",
        help="also build the gtest-based C++ API tests (tests/ subdirectory, "
             "adds -DBUILD_TESTING=ON and builds the cjprof_gtest target). "
             "Requires GTest to be installed (find_package(GTest REQUIRED)). "
             "The tests are NOT run here — run them later via ctest. "
             "Independent of --code-coverage.")
    parser_build.add_argument(
        "cmake_args",
        nargs=argparse.REMAINDER,
        help="other arguments directly passed to CMake, " "please pass it after a '--'",
    )
    parser_build.set_defaults(func=build)

    parser_install = subparsers.add_parser("install", help="install targets")
    parser_install.add_argument(
        "--prefix", dest="install_prefix", help="target install prefix"
    )
    parser_install.set_defaults(func=install)

    parser_clean = subparsers.add_parser("clean", help="clean build")
    parser_clean.add_argument(
        "--skip-jni",
        action="store_true",
        help="skip JNI/Java bridge clean")
    parser_clean.set_defaults(func=clean)

    args = parser.parse_args()
    if not hasattr(args, "func"):
        args = parser.parse_args(["build"] + sys.argv[1:])

    args.func(args)
    if FAIL_CASE_NUM != 0:
        exit(1)


if __name__ == "__main__":
    LOG = init_log("root")
    os.environ["LANG"] = "C.UTF-8"
    setup_compiler()
    main()
