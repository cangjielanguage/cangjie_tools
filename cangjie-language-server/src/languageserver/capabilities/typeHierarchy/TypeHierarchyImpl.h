// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_TYPEHIERARCHYIMPL_H
#define LSPSERVER_TYPEHIERARCHYIMPL_H

#include "../../../json-rpc/Protocol.h"
#include "../../ArkAST.h"
#include "../../common/FindDeclUsage.h"
#include "../../logger/Logger.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/AST/Symbol.h"
#include "cangjie/Lex/Token.h"
#include "cangjie/Modules/ImportManager.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/Utils/FileUtil.h"

namespace ark {
class TypeHierarchyImpl {
public:
    static std::string curFilePath;
    static Position curPos;

    static void FindTypeHierarchyImpl(const ArkAST &ast, TypeHierarchyItem &result, Cangjie::Position pos);

    static TypeHierarchyItem TypeHierarchyFrom(Ptr<const Decl> decl);

    static void FindSuperTypesImpl(std::vector<TypeHierarchyItem> &results, const TypeHierarchyItem &hierarchyItem);

    static void FindSubTypesImpl(std::vector<TypeHierarchyItem> &results, const TypeHierarchyItem &hierarchyItem);
};
} // namespace ark

#endif // LSPSERVER_TYPEHIERARCHYIMPL_H
