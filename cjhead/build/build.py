# Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

import os
import subprocess
import sys
import shutil
import configparser
import argparse


def load_config(config_path="./config/config.ini"):
    if not os.path.exists(config_path):
        print(f"❌ Configuration file not found: {config_path}")
        sys.exit(1)

    try:
        config = configparser.ConfigParser()
        config.read(config_path)
        print(f"✅ Loaded configuration from {config_path}")

        root_raw = config["project"]["root"].strip()

        if len(root_raw) >= 2 and root_raw[0] in ('"', "'") and root_raw[-1] == root_raw[0]:
            root_raw = root_raw[1:-1]

        script_dir = os.path.dirname(os.path.abspath(__file__))

        PROJECT_ROOT = os.path.normpath(os.path.join(script_dir, root_raw))

        if not os.path.isabs(PROJECT_ROOT):
            print(f"❌ Project root must be an absolute path after resolving: '{PROJECT_ROOT}'")
            sys.exit(1)

        if not os.path.exists(PROJECT_ROOT):
            print(f"📁 Project root does not exist: {PROJECT_ROOT}")
            create = input("Would you like to create it? (y/N): ").strip().lower()
            if create == 'y':
                os.makedirs(PROJECT_ROOT, exist_ok=True)
                print(f"✅ Created directory: {PROJECT_ROOT}")
            else:
                print("❌ Aborted due to missing root directory.")
                sys.exit(1)

        if not os.access(PROJECT_ROOT, os.W_OK):
            print(f"⚠️ Warning: Project root is not writable: {PROJECT_ROOT}")
            confirm = input("Continue anyway? (y/N): ").strip().lower()
            if confirm != 'y':
                sys.exit(1)

        config["project"]["root"] = PROJECT_ROOT

        print(f"📁 Project root resolved to: {PROJECT_ROOT}")
        return config

    except Exception as e:
        print(f"❌ Failed to parse or validate config file: {e}")
        sys.exit(1)


CONFIG = load_config()


# ================== Extract config values ==================
PROJECT_ROOT = CONFIG["project"]["root"]

CANGJIE_DIR = os.path.join(PROJECT_ROOT, CONFIG["paths"]["cangjie_dir"])
BUILD_DIR = os.path.join(PROJECT_ROOT, CONFIG["paths"]["build_dir"])
CANGJIE_BUILD_SCRIPT = os.path.join(CANGJIE_DIR, CONFIG["paths"]["cangjie_build_script"])
ENVSETUP_SH = os.path.join(CANGJIE_DIR, CONFIG["paths"]["envsetup_sh"])
CJHEAD_SRC_DIR = os.path.join(PROJECT_ROOT, CONFIG["paths"]["cjhead_src_dir"])
CANGJIE_OUTPUT_DIR = os.path.join(PROJECT_ROOT, CONFIG["paths"]["cangjie_output_dir"])
CANGJIE_REPO = CONFIG["cangjie"]["repo"]
CANGJIE_BRANCH = CONFIG["cangjie"]["branch"].strip()

# Build options
BUILD_OPTIONS = []
if CONFIG.getboolean("build", "no_tests"):
    BUILD_OPTIONS.append("--no-tests")
if CONFIG.getboolean("build", "cjnative"):
    BUILD_OPTIONS.append("--cjnative")
BUILD_OPTIONS.extend(["-t", CONFIG["build"]["target"]])
if CONFIG.getboolean("build", "enable_assert"):
    BUILD_OPTIONS.append("--enable-assert")


# ===================================================

def run_command(cmd, cwd=None, shell=False, check=True):
    """Run command with logging and error handling."""
    cmd_str = ' '.join(cmd) if isinstance(cmd, list) else cmd
    print(f"▶ Running: {cmd_str}")

    result = subprocess.run(
        cmd,
        cwd=cwd,
        shell=shell,
        capture_output=True,
        text=True,
        encoding='utf-8',
        errors='replace'
    )

    if check and result.returncode != 0:
        print("❌ Error occurred:")
        if result.stderr:
            print(result.stderr.strip())
        sys.exit(1)

    if result.stdout:
        print(result.stdout.strip())

    if result.stderr:
        print(result.stderr.strip())

    return result


def ensure_cangjie_installed():
    if not os.path.exists(CANGJIE_DIR):
        print("🔧 First-time setup: Cloning Cangjie repository...")
        run_command(["git", "clone", "-b", CANGJIE_BRANCH, CANGJIE_REPO, CANGJIE_DIR])
    else:
        print("✅ Cangjie repository already exists, skipping clone.")
        return True

    if os.path.exists(ENVSETUP_SH):
        print("✅ Cangjie is already installed, skipping rebuild.")
        return True


def build_cangjie():
    if os.path.exists(CANGJIE_OUTPUT_DIR):
        return
    print("🔧 Building Cangjie...")

    os.chdir(CANGJIE_DIR)
    run_command(["./build.py", "build"] + BUILD_OPTIONS)
    run_command(["./build.py", "install"])
    os.chdir(PROJECT_ROOT)

    print("✅ Cangjie built and installed successfully!")
    print("💡 Please run: source {}".format(os.path.abspath(ENVSETUP_SH)))
    return True


def build_cjhead():
    print("🏗️ Starting to build cjhead...")
    if os.path.exists(BUILD_DIR):
        print(f"🗑️ Removing old build directory: {BUILD_DIR}")
        shutil.rmtree(BUILD_DIR)
    os.makedirs(BUILD_DIR, exist_ok=True)

    os.chdir(BUILD_DIR)
    run_command(["cmake", ".."], check=True)
    run_command(["make", "-j32"], check=True)

    bin_path = os.path.join(BUILD_DIR, "bin", "cjhead")
    if os.path.exists(bin_path):
        print(f"✂️ Stripping symbols from {bin_path}...")
        run_command(["strip", "--strip-all", bin_path])
        print(f"✅ Symbol stripping completed.")
    else:
        print("❌ Build failed: cjhead executable not found.")
        sys.exit(1)

def recreate_dir(path):
    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--kernel-path", default="", help="Path to copy kernel files into Cangjie directory (optional)")
    args = parser.parse_args()

    if args.kernel_path:
        print(f"🔧 Copying kernel files from {args.kernel_path} to {CANGJIE_DIR}...")
        recreate_dir(CANGJIE_DIR)
        if not os.path.exists(args.kernel_path):
            print(f"❌ Source path does not exist: {args.kernel_path}")
            sys.exit(1)
        if not os.path.isdir(args.kernel_path):
            print(f"❌ Source is not a directory: {args.kernel_path}")
            sys.exit(1)

        for item in os.listdir(args.kernel_path):
            src_path = os.path.join(args.kernel_path, item)
            dst_path = os.path.join(CANGJIE_DIR, item)
            if os.path.isdir(src_path):
                shutil.copytree(src_path, dst_path, dirs_exist_ok=True)
            else:
                shutil.copy2(src_path, dst_path)
        print(f"✅ Successfully added kernel files to {CANGJIE_DIR}")
    else:
        print("🔧 Cangjie installation will proceed normally.")
        ensure_cangjie_installed()

    build_cangjie()
    build_cjhead()
    print("\n✅ All done!")


if __name__ == "__main__":
    main()
