// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_BREAKPOINTSIMPL_H
#define LSPSERVER_BREAKPOINTSIMPL_H

#include "../../ArkAST.h"
#include "../../../json-rpc/Protocol.h"
#include "../../ArkASTWorker.h"

namespace ark {
class BreakpointsImpl {
public:
    static std::string curFilePath;

    static void Breakpoints(const ArkAST &ast, std::set<BreakpointLocation> &result);

    static void HandleBlockExit(std::set<BreakpointLocation> &result, Ptr<const Block> block);
};
}

#endif // LSPSERVER_BREAKPOINTSIMPL_H
