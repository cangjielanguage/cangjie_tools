// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CJHeadCompilerInvocation.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include "cangjie/Utils/FileUtil.h"

namespace CJHead {
#ifdef _WIN32
const std::string CJC_PATH = "/bin/cjc.exe";
#else
const std::string CJC_PATH = "/bin/cjc";
#endif

CJHeadCompilerInvocation::CJHeadCompilerInvocation() : CompilerInvocation()
{
    globalOptions.enableAddCommentToAst = true;
}

void CJHeadCompilerInvocation::SetCompilePackage(const bool &flag)
{
    globalOptions.compilePackage = flag;
}

void CJHeadCompilerInvocation::SetCangjieHome(const std::unordered_map<std::string, std::string> &environment_vars)
{
    if (environment_vars.find(CANGJIE_HOME) == environment_vars.end()) {
        std::cerr << "can not find CANGJIE_HOME, please setup CANGJIE_HOME\n";
        exit(1);
    }
    auto cangjie_home = FileUtil::GetAbsPath(environment_vars.at(CANGJIE_HOME));
    if (!cangjie_home.has_value()) {
        std::cerr << "Can not find realpath of CANGJIE_HOME, please setup CANGJIE_HOME\n";
        exit(1);
    }
    globalOptions.cangjieHome = cangjie_home.value();
    globalOptions.executablePath = FileUtil::JoinPath(cangjie_home.value(), CJC_PATH);
}

void CJHeadCompilerInvocation::SetPackagePath(const std::string &path)
{
    globalOptions.packagePaths.emplace_back(path);
}

void CJHeadCompilerInvocation::SetSrcFiles(const std::vector<std::string> &src_files)
{
    globalOptions.srcFiles = src_files;
}

void CJHeadCompilerInvocation::SetSrcFiles(const std::string &src_file)
{
    globalOptions.srcFiles.emplace_back(src_file);
}

void CJHeadCompilerInvocation::SetImportPath(const std::string &import_path)
{
    globalOptions.importPaths.emplace_back(import_path);
}
}  // namespace CJHead
