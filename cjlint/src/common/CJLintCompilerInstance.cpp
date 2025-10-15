// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CJLintCompilerInstance.h"

using namespace Cangjie;
using namespace Cangjie::CodeCheck;
using namespace AST;

#ifdef _WIN32
const std::string EXTENSION = ".dll";
const std::string LIBTYPE = "dylib";
const std::string EMPTY_DEVICE = "NUL";
#else
const std::string EXTENSION = ".so";
const std::string LIBTYPE = "dylib";
const std::string EMPTY_DEVICE = "/dev/null";
#endif

bool CJLintCompilerInstance::PerformImportPackage()
{
    std::vector<std::string> importPaths;
    for (auto it = srcPkgs.begin(); it != srcPkgs.end();) {
        auto srcPkg = it->get();
        if (!srcPkg->isMacroPackage) {
            ++it;
            continue;
        }
        auto macroPkgName = FileUtil::SplitStr(srcPkg->fullPackageName, '.');
        std::string package = FileUtil::JoinPath(invocation.globalOptions.moduleSrcPath, macroPkgName.back());
        std::string output;
        if (FileUtil::GetAbsPath(package).has_value()) {
            output = package + "/lib-macro_" + srcPkg->fullPackageName + EXTENSION;
        } else {
            ++it;
            continue;
        }
        int res = 0;
#ifdef _WIN32
        if (!FileUtil::FileExist(output.c_str()) || remove(output.c_str()) == 0) {
#endif
            std::string cmd = "cjc -p " + package + " --compile-macro -o " + package + " > " + EMPTY_DEVICE + " 2>&1";
            res = system(cmd.c_str());
#ifdef _WIN32
        }
#endif
        if (res != 0) {
            (void)diag.Diagnose(DiagKind::macro_expand_failed);
            return false;
        }
        (void)macroPath.insert(package + PATH_SEPARATOR + srcPkg->fullPackageName);

        importPaths.emplace_back(package);
        it = srcPkgs.erase(it);
    }
    for (auto& importPath : importPaths) {
        invocation.globalOptions.importPaths.emplace_back(importPath);
    }
    return CompilerInstance::PerformImportPackage();
}

bool CJLintCompilerInstance::Compile(CompileStage stage)
{
    if ((this->currentStage == CompileStage::PARSE) || this->currentStage > stage) {
        bool ret = CompilerInstance::Compile(stage);
        if (ret) {
            this->currentStage = static_cast<CompileStage>(static_cast<int>(stage) + 1);
        }
        return ret;
    }
    if ((this->currentStage == CompileStage::PARSE) && !InitCompilerInstance()) {
        diag.ReportErrorAndWarningCount();
        return false;
    }
    for (int i = static_cast<int>(this->currentStage); i <= static_cast<int>(stage); i++) {
        Cangjie::ICE::TriggerPointSetter iceSetter(static_cast<CompileStage>(i));
        if (!performMap[static_cast<CompileStage>(i)](this)) {
            diag.ReportErrorAndWarningCount();
            return false;
        }
    }
    Cangjie::ICE::TriggerPointSetter iceSetter(stage);
    diag.ReportErrorAndWarningCount();
    this->currentStage = static_cast<CompileStage>(static_cast<int>(stage) + 1);
    return true;
}
