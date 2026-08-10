#!/bin/bash
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie Project, licensed under Apache-2.0
# with Runtime Library Exception.
#
# See https://cangjie-lang.cn/pages/LICENSE for license information.

set -e
echo "Command execution start."
curdir=$(pwd)

sdkBaseVersion="$1"
sdkCurVersion="$2"

if [ -d "$sdkBaseVersion" ]; then
    echo "The directory '$sdkBaseVersion' exist"
else
    echo "The directory '$sdkBaseVersion' does not exist. Exiting with error."
    exit 1
fi

if [ -d "$sdkCurVersion" ]; then
    echo "The directory '$sdkCurVersion' exist"
else
    echo "The directory '$sdkCurVersion' does not exist. Exiting with error."
    exit 1
fi

# cjo type
suffix=".cjo"

# Set directory path and file extension
linux_type="$3"
dir1="$sdkBaseVersion/modules/linux_${linux_type}_cjnative"
if [ -d "$dir1" ]; then
    echo "The directory '$dir1' exist"
else
    echo "The directory '$dir1' does not exist."
    dir1="$sdkBaseVersion/modules/linux_${linux_type}_llvm"
    if [ -d "$dir1" ]; then
        echo "The directory '$dir1' exist"
    else
        echo "The directory '$dir1' does not exist. Exiting with error."
        exit 1
    fi
fi

dir2="$sdkCurVersion/modules/linux_${linux_type}_cjnative"
if [ -d "$dir2" ]; then
    echo "The directory '$dir2' exist"
else
    echo "The directory '$dir2' does not exist."
    if [ -d "$dir2" ]; then
        dir2="$sdkCurVersion/modules/linux_${linux_type}_llvm"
        echo "The directory '$dir2' exist"
    else
        echo "The directory '$dir2' does not exist. Exiting with error."
        exit 1
    fi
fi

# Set log file path
log_file="$curdir/result_cjcompat_libs.log"

# Get a list of relative paths to all files with a specified suffix in two directories
find "$dir1" -type f -name "*$suffix" | sed "s|^$dir1/||" | sort > $curdir/dir1_files.txt
find "$dir2" -type f -name "*$suffix" | sed "s|^$dir2/||" | sort > $curdir/dir2_files.txt

# Find the file names that exist in both directories
comm -12 $curdir/dir1_files.txt $curdir/dir2_files.txt | while read -r filename; do
    echo "filename: $filename" >> $log_file
    file1="$dir1/$filename"
    file2="$dir2/$filename"

    # Check whether the file exists
    if [ -f "$file1" ] && [ -f "$file2" ]; then
        echo "Found matching files:" >> $log_file
        echo "  $file1" >> $log_file
        echo "  $file2" >> $log_file

        echo "Running command: cjcompat $filename" >> $log_file
        # Execute commands concurrently and append the results to a log file
        # cjcompat --old "$file1" --new "$file2" >> $log_file 2>&1 &

        # Execute commands in sequence and append the results to the log file
        cjcompat --old $file1 --new $file2 >> $log_file 2>&1
    fi
    echo "---------------------------------------------" >> $log_file
done

# Clear temp files
rm -f $curdir/dir1_files.txt $curdir/dir2_files.txt

cat -n ${log_file}
echo "Command execution completed. Results logged to $log_file"
if grep -n "Incompatible" "$log_file"; then
    echo "Text 'Incompatible' found in file '$log_file'. Exiting with error."
    exit 0
else
    echo "Text 'Incompatible' not found in file '$log_file'. Exiting normally."
    exit 0
fi
