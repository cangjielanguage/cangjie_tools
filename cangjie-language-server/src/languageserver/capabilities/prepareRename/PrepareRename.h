// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_PREPARERENAME_H
#define LSPSERVER_PREPARERENAME_H


#include "../../ArkASTWorker.h"

namespace ark {
class PrepareRename {
public:
    static Range PrepareImpl(const ArkAST &ast, Cangjie::Position pos, MessageErrorDetail &errorInfo);
private:
    static bool IsNotFromLibInherit(Ptr<Decl> decl);
    static bool IsFromMacroCallFile(Ptr<Decl> decl);
};
} // namespace ark

#endif // LSPSERVER_PREPARERENAME_H
