#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
# This source file is part of the Cangjie Project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

import os
import sys
import shutil
import subprocess
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent.resolve()
BUILD_DIR = SCRIPT_DIR / "build"
JAVA_OUT = SCRIPT_DIR / "out"
DIST_DIR = SCRIPT_DIR / "dist"

RED = '\033[0;31m'
GREEN = '\033[0;32m'
YELLOW = '\033[1;33m'
NC = '\033[0m'

LIBS = ["libcjprof-jni.so", "cjprof-jni.dll", "libcjprof-jni.dylib"]

def echo_info(msg):
    print(f"{GREEN}[INFO]{NC} {msg}")


def echo_warn(msg):
    print(f"{YELLOW}[WARN]{NC} {msg}")


def echo_error(msg):
    print(f"{RED}[ERROR]{NC} {msg}", file=sys.stderr)

def find_jni_headers_and_copy(stdinput):
    jni_include_paths = []
    for line in stdinput.splitlines():
        if "Found JNI:" in line:
            paths = line.split("Found JNI:")[1].strip()
            jni_include_paths = paths.split(";")

    jni_h_directory = None
    for inc_dir in jni_include_paths:
        inc_path = Path(inc_dir)
        if inc_path.exists() and (inc_path / "jni.h").is_file():
            jni_h_directory = inc_path
            break

    if not jni_h_directory:
        raise FileNotFoundError(f"Cannot find jni.h, search paths: {jni_include_paths}")

    win32_target_dir = jni_h_directory / "win32"
    target_file = win32_target_dir / "jni_md.h"
    if target_file.is_file():
        return ""
    win32_target_dir.mkdir(exist_ok=True)

    source_file = SCRIPT_DIR / "cross_win" / "jni_md.h"
    if not source_file.is_file():
        raise FileNotFoundError(f"source_file doesn't exist: {source_file}")
    shutil.copy2(source_file, target_file)
    return win32_target_dir

def clean():
    echo_info("Cleaning build directories...")
    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
    if JAVA_OUT.exists():
        shutil.rmtree(JAVA_OUT)
    echo_info("Clean completed")


def build_jni(debug=True, target="native"):
    build_type = "Debug" if debug else "Release"
    echo_info(f"Building JNI library ({build_type})...")

    BUILD_DIR.mkdir(exist_ok=True)

    if not shutil.which("cmake"):
        echo_error("cmake not found")
        sys.exit(1)
    make_cmd = "mingw32-make" if sys.platform == "win32" else "make"
    if not shutil.which(make_cmd):
        echo_error(f"{make_cmd} not found")
        sys.exit(1)

    win32_target_dir = ""
    if target == "windows-x86_64":
        result = subprocess.run([
            "cmake",
            "-DCMAKE_BUILD_TYPE=" + build_type,
            "-DCMAKE_SYSTEM_NAME=" + "Windows",
            "-DCMAKE_C_COMPILER=" + "x86_64-w64-mingw32-gcc",
            "-DCMAKE_CXX_COMPILER=" + "x86_64-w64-mingw32-g++",
            "-DSTRIP_LIB_PREFIX=" + "ON",
            "..",
            "-DCMAKE_LIBRARY_PATH=../../build/build/src"
            ], cwd=BUILD_DIR, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        echo_info(f"{result.stdout}")
        echo_info(f"{result.stderr}")
        win32_target_dir = find_jni_headers_and_copy(result.stdout)
    else:
        cmake_args = ["cmake", "-DCMAKE_BUILD_TYPE=" + build_type, "..", "-DCMAKE_LIBRARY_PATH=../../build/build/src"]
        if sys.platform == "win32":
            cmake_args += ["-G", "MinGW Makefiles", "-DSTRIP_LIB_PREFIX=" + "ON"]
        subprocess.run(cmake_args, cwd=BUILD_DIR, check=True)
    try:
        subprocess.run([make_cmd, f"-j{os.cpu_count() or 1}"], cwd=BUILD_DIR, check=True)
    except subprocess.CalledProcessError as e:
        if win32_target_dir != "":
            shutil.rmtree(win32_target_dir)
        raise RuntimeError(f"{make_cmd} error: {e.stderr}") from e
    if win32_target_dir != "":
        shutil.rmtree(win32_target_dir)
    echo_info("JNI library built successfully")


def compile_java():
    echo_info("Compiling Java sources...")

    JAVA_OUT.mkdir(exist_ok=True)

    if not shutil.which("javac"):
        echo_error("javac not found")
        sys.exit(1)

    java_model_dir = SCRIPT_DIR / "java" / "com" / "cjprof" / "jni" / "model"
    java_main_dir = SCRIPT_DIR / "java" / "com" / "cjprof" / "jni"

    model_files = [
        java_model_dir / "HeapSnapshot.java",
        java_model_dir / "BaseNode.java",
        java_model_dir / "InstanceNode.java",
        java_model_dir / "InstanceDiffNode.java",
        java_model_dir / "ConstructorNode.java",
        java_model_dir / "ConstructorDiffNode.java",
        java_model_dir / "Frame.java",
        java_model_dir / "ThreadInfo.java",
        java_main_dir / "CjprofException.java",
    ]
    subprocess.run(["javac", "-cp", "lombok-1.18.30.jar", "-d", str(JAVA_OUT)] + [str(f) for f in model_files], check=True)

    subprocess.run(
        ["javac", "-cp", str(JAVA_OUT), "-d", str(JAVA_OUT), str(java_main_dir / "Cjprof.java")],
        check=True
    )

    subprocess.run(
        ["javac", "-cp", str(JAVA_OUT), "-d", str(JAVA_OUT), str(java_main_dir / "CjprofDemo.java")],
        check=True
    )

    echo_info("Java compilation completed")


def run_demo():
    echo_info("Running CjprofDemo...")

    jni_lib_path = None

    for lib in LIBS:
        lib_path = BUILD_DIR / lib
        if lib_path.exists():
            jni_lib_path = str(BUILD_DIR)
            break

    if jni_lib_path is None:
        echo_error(f"JNI library not found in {BUILD_DIR}")
        if BUILD_DIR.exists():
            for f in BUILD_DIR.iterdir():
                print(f"  {f.name}")
        sys.exit(1)

    if sys.platform == "win32":
        env_path = os.environ.get("PATH", "")
        os.environ["PATH"] = f"{jni_lib_path};{env_path}" if env_path else jni_lib_path
    else:
        env_path = os.environ.get("LD_LIBRARY_PATH", "")
        os.environ["LD_LIBRARY_PATH"] = f"{jni_lib_path}:{env_path}" if env_path else jni_lib_path

    subprocess.run(
        ["java", "-Djava.library.path=" + jni_lib_path, "-cp", str(JAVA_OUT), "com.cjprof.jni.CjprofDemo"],
        check=True
    )


def build_and_run():
    clean()
    build_jni()
    compile_java()
    run_demo()

def install():
    echo_info("Installing distribution files...")

    if not any((BUILD_DIR / lib).exists() for lib in LIBS):
        build_jni()

    if DIST_DIR.exists():
        shutil.rmtree(DIST_DIR)
    DIST_DIR.mkdir(exist_ok=True)

    for lib in LIBS:
        src = BUILD_DIR / lib
        if src.exists():
            shutil.copy2(src, DIST_DIR / lib)
            echo_info(f"  Copied {lib}")

    echo_info(f"Installation completed: {DIST_DIR}")
    echo_info("")
    echo_info("Distribution contents:")
    for f in DIST_DIR.iterdir():
        if f.is_dir():
            echo_info(f"  {f.name}/")
            for sub in f.iterdir():
                echo_info(f"    {sub.name}")
        else:
            echo_info(f"  {f.name}")

def usage():
    print("Usage: python build.py [command] [-t debug|release]")
    print("")
    print("Commands:")
    print("  clean          Clean build directories")
    print("  build          Build JNI library only")
    print("  compile        Compile Java sources only")
    print("  run            Run demo (requires build first)")
    print("  install        Create distribution package")
    print("")
    print("Options:")
    print("  -t debug       Build in debug mode (only for build/all)")
    print("  -t release     Build in release mode (default)")
    print("")
    print("Examples:")
    print("  python build.py                      # Build and run (release)")
    print("  python build.py build                # Build JNI library only (release)")
    print("  python build.py build -t debug       # Build JNI library (debug)")
    print("  python build.py install             # Create distribution package")

def main():
    debug = True
    cmd = "all"
    target = "native"

    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == "-t" and i + 1 < len(sys.argv):
            i += 1
            build_type = sys.argv[i]
            if build_type == "debug":
                debug = True
            elif build_type == "release":
                debug = False
            else:
                echo_error(f"Unknown build type: {build_type}")
                usage()
                sys.exit(1)
        elif arg == "--target":
            i += 1
            if i >= len(sys.argv):
                echo_error("--target requires a value (e.g. windows-x86_64)")
                usage()
                sys.exit(1)
            target = sys.argv[i]
        elif not arg.startswith("-"):
            cmd = arg
        i += 1

    if cmd == "all":
        def build_and_run_with_release():
            clean()
            build_jni(False)
            compile_java()
            run_demo()
        build_and_run_with_release()
    elif cmd == "build":
        build_jni(debug, target)
    elif cmd == "install":
        install()
    elif cmd == "clean":
        clean()
    elif cmd == "compile":
        compile_java()
    elif cmd == "run":
        run_demo()
    else:
        usage()
        sys.exit(1)


if __name__ == "__main__":
    main()
