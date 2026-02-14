#!/bin/sh

# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# This source file is part of the Cangjie project, licensed under Apache-2.0
# with Runtime Library Exception.
# 
# See https://cangjie-lang.cn/pages/LICENSE for license information.

# 本脚本用于CI调用，验证测试用例是否能编译成功，只在linux上运行。

if [ -z $CANGJIE_HOME ]; then
    echo "error: cannot find CANGJIE_HOME, please make sure cangjie sdk is configured"
    exit 1
fi

ohos_dir=$CANGJIE_HOME/../api/lib/linux_ohos_x86_64_cjnative/ohos
test_cases_dir="$(dirname $(readlink -f "$0"))/expected/my_module"
cd $test_cases_dir

for file in $(ls ./); do
    if [ ! -f $file ]; then
        continue
    fi
    if [ $file != ark_api_call_async.cj -a $file != business_exception.cj -a $file != callback_manager.cj ]; then
        if [ $file == import.cj ]; then
            compilefile="$file inheritances.cj export_alias.cj"
        elif [ $file == inheritances.cj ]; then
            compilefile="$file super_interface.cj"
        elif [ $file == value_conversion.cj ]; then
            compilefile="$file export_alias.cj"
        elif [ $file == class.cj ]; then
            compilefile="$file interface.cj"
        else
            compilefile=$file
        fi
        cjc $compilefile ark_api_call_async.cj  business_exception.cj callback_manager.cj --output-type=staticlib --import-path $ohos_dir --error-count-limit all --warn-off all
        if [ $? -ne 0 ]; then
            echo "compile $compilefile error"
            exit 1
        fi
    fi
done;

cd -