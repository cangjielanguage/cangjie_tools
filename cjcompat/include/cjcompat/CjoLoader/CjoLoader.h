// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJO_LOADER_CJO_LOADER_H
#define CJO_LOADER_CJO_LOADER_H

#include "cangjie/AST/Node.h"
#include "cangjie/Basic/DiagnosticEngine.h"
#include "cangjie/Basic/SourceManager.h"
#include "cangjie/Option/Option.h"
#include "cangjie/Sema/TypeManager.h"
#include "cangjie/Frontend/CompilerInstance.h"

class CjoLoader {
public:
    CjoLoader(std::string cjoPath, std::string modulesPath,
        std::vector<std::string> cangjiePaths, std::vector<std::string> importPaths)
        : cjoPath{cjoPath}, modulesPath{modulesPath}, ci{loadInvocation, diagEngine}
    {
        loadInvocation.globalOptions.environment.cangjiePaths = cangjiePaths;
        loadInvocation.globalOptions.importPaths = importPaths;
        diagEngine.SetSourceManager(&ci.GetSourceManager());
    }

    bool Load();

    const Cangjie::AST::Package* GetPackage() const;

    Cangjie::SourceManager& GetSourceManager();
    Cangjie::TypeManager& GetTypeManager();

private:
    Cangjie::DiagnosticEngine diagEngine{};
    Cangjie::CompilerInvocation loadInvocation{};
    static const std::string PACKAGENAME;
    std::string cjoPath;
    std::string modulesPath;
    Cangjie::CompilerInstance ci;
    Cangjie::AST::Package* package{};
};

#endif // CJO_LOADER_CJO_LOADER_H
