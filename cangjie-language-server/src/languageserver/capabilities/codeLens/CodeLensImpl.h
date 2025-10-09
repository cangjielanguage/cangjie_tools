// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_CODELENSIMPL_H
#define LSPSERVER_CODELENSIMPL_H

#include "../../ArkAST.h"
#include "../../../json-rpc/Protocol.h"
#include "../../ArkASTWorker.h"

namespace ark {
    class CodeLensImpl {
    public:
        static std::string curFilePath;

        static void GetCodeLens(const ArkAST &ast, std::vector<CodeLens> &result);
    };
}

#endif // LSPSERVER_CODELENSIMPL_H
