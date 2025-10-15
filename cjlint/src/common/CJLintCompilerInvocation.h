// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_CJLINTCOMPILERINVOCATION_H
#define CANGJIECODECHECK_CJLINTCOMPILERINVOCATION_H

#include "cangjie/Macro/InvokeUtil.h"
#include "common/CJLintCompilerInstance.h"

namespace Cangjie::CodeCheck {
#ifdef _WIN32
const std::string CJC_PATH = "/bin/cjc.exe";
#else
const std::string CJC_PATH = "/bin/cjc";
#endif
enum class CompilerInvocationType {
    AST,      // no type
    TYPEDAST, // typed AST
    CHIRTYPE
};

class CJLintCompilerInvocation {
public:
    static CJLintCompilerInvocation &GetInstance()
    {
        static CJLintCompilerInvocation instance;
        return instance;
    }
    std::unique_ptr<CJLintCompilerInstance> PrePareCompilerInstance(DiagnosticEngine& diag);
    void DesInitRuntime();
private:
    CJLintCompilerInvocation();
    ~CJLintCompilerInvocation();
    CJLintCompilerInvocation(const CJLintCompilerInvocation&);
    CJLintCompilerInvocation& operator=(const CJLintCompilerInvocation&);
    std::unique_ptr<CompilerInvocation> invocation;
    std::unique_ptr<RuntimeInit> runtimeInit;
};
}
#endif // CANGJIECODECHECK_CJLINTCOMPILERINVOCATION_H
