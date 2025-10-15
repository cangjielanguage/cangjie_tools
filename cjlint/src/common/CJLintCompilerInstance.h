// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJLINT_COMPILER_INSTANCE_H
#define CJLINT_COMPILER_INSTANCE_H

#include "cangjie/Basic/DiagnosticEngine.h"
#include "cangjie/Frontend/CompilerInstance.h"
#include "cangjie/Frontend/CompilerInvocation.h"
#include "common/ConfigContext.h"

namespace Cangjie::CodeCheck {
class CJLintCompilerInstance : public CompilerInstance {
public:
    CJLintCompilerInstance(CompilerInvocation& invocation, DiagnosticEngine& diag) : CompilerInstance(invocation, diag)
    {
        buildTrie = false;
        releaseCHIRMemory = false;
        needToOptString = true;
        needToOptGenericDecl = true;
        isCJLint = true;
    }
    bool PerformImportPackage() override;
    bool Compile(CompileStage stage) override;

    std::set<std::string> macroPath;

private:
    CompileStage currentStage = CompileStage::PARSE;
};
} // namespace Cangjie::CodeCheck

#endif // CJLINT_COMPILER_INSTANCE_H
