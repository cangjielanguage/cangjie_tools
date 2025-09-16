// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LOCATEDEFINITION4IMPORT_H
#define LOCATEDEFINITION4IMPORT_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "../../CompilerCangjieProject.h"

namespace ark {

class LocateDefinition4Import {
public:
    static void getImportDecl(const std::vector<Symbol *> &syms,
                              const ArkAST &ast,
                              Position pos,
                              Ptr<Decl> &decl);

private:
    static Ptr<Decl> getDecl(const ArkAST &ast, Position pos, File *fileNode);
};

};


#endif // LOCATEDEFINITION4IMPORT_H