// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/TryExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void TryExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto tryExpr = As<ASTKind::TRY_EXPR>(node);
    AddTryExpr(doc, *tryExpr, level);
}
void TryExprFormatter::AddTryExpr(Doc& doc, const Cangjie::AST::TryExpr& tryExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    // try-with-resource and normal try
    AddIsTryWithResource(doc, tryExpr, level);

    // tryBlock
    if (tryExpr.tryBlock) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(tryExpr.tryBlock.get(), level));
    }

    // catch block
    for (size_t i = 0; i < tryExpr.catchPosVector.size(); ++i) {
        doc.members.emplace_back(DocType::STRING, level, " catch ");
        doc.members.emplace_back(DocType::STRING, level, "(");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(tryExpr.catchPatterns.at(i).get(), level));
        doc.members.emplace_back(DocType::STRING, level, ")");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(tryExpr.catchBlocks.at(i).get(), level));
    }

    // finallyBlock
    if (tryExpr.finallyBlock) {
        doc.members.emplace_back(DocType::STRING, level, " finally");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(tryExpr.finallyBlock.get(), level));
    }
}
void TryExprFormatter::AddIsTryWithResource(Doc& doc, const Cangjie::AST::TryExpr& tryExpr, int level)
{
    if (!tryExpr.resourceSpec.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "try (");
        FuncOptions funcOptions;
        funcOptions.patternOrEnum = true;
        for (auto& resDecl : tryExpr.resourceSpec) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(resDecl.get(), level, funcOptions));
            if (resDecl != tryExpr.resourceSpec.back()) {
                doc.members.emplace_back(DocType::STRING, level, ", ");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, ")");
    } else {
        doc.members.emplace_back(DocType::STRING, level, "try");
    }
}
} // namespace Cangjie::Format