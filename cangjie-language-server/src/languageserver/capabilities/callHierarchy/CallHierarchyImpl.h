// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_CALLHIERARCHYIMPL_H
#define LSPSERVER_CALLHIERARCHYIMPL_H

#include "../../ArkAST.h"
#include "../../../json-rpc/Protocol.h"
#include "../../CompilerCangjieProject.h"

namespace ark {
    class CallHierarchyImpl {
    public:
        static std::string curFilePath;

        static void FindCallHierarchyImpl(const ArkAST &ast, CallHierarchyItem &result, Position pos);

        // show caller
        static void
        FindOnIncomingCallsImpl(std::vector<CallHierarchyIncomingCall> &results,
                                const CallHierarchyItem &callHierarchyItem);

        // show callee
        static void
        FindOnOutgoingCallsImpl(std::vector<CallHierarchyOutgoingCall> &results,
                                const CallHierarchyItem &callHierarchyItem);
    };
}


#endif // LSPSERVER_CALLHIERARCHYIMPL_H
