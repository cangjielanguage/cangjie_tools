// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJHEAD_COMPILER_INVOCATION_H
#define CJHEAD_COMPILER_INVOCATION_H
#include <string>
#include <vector>
#include <unordered_map>
#include "cangjie/Frontend/CompilerInvocation.h"

using namespace Cangjie;
namespace CJHead {
class CJHeadCompilerInvocation : public CompilerInvocation {
public:
    CJHeadCompilerInvocation();

    void SetCangjieHome(const std::unordered_map<std::string, std::string> &environment_vars);

    void SetCompilePackage(const bool &flag);

    void SetPackagePath(const std::string &path);

    void SetSrcFiles(const std::vector<std::string> &src_files);

    void SetSrcFiles(const std::string &src_file);

    void SetImportPath(const std::string &import_path);
};
}  // namespace CJHead

#endif