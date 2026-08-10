// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/Cjcompat.h"
#include "cjcompat/Option/Option.h"


using namespace CjComplienceChecker;

int main(int argc, char **argv, const char **envp)
{
    CjComplienceChecker::Options options;

    auto ret = HandleOptions(argc, argv, envp, options);
    if (ret == END) {
        return OK;
    }
    if (ret == ERR) {
        return ERR;
    }

    return CJcompat(options);
}
