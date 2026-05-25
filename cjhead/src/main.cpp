// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <iostream>
#include "cangjie/Utils/Utils.h"
#include "FileHandler.h"

using namespace CJHead;

int main(int argc, char **argv, const char **envp)
{
    FileHandler file_handler(
        argc, CJHeadUtil::ConvertArgv(argc, argv), Cangjie::Utils::StringifyEnvironmentPointer(envp));
    if (!file_handler.ParseArguments()) {
        return 0;
    }
    if (file_handler.Run()) {
        std::cout << "\033[31m\nexecute successfully\t◕‿◕\n\033[0m";
    }
}