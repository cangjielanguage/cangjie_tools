// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.var_test.auto_import_completion.pkg1

func auto_import_func1(aa: (String) -> String) {
    ""
}

public var auto_import_var1: ((String) -> String) -> String = auto_import_func1
