// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/SpawnExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void SpawnExprFormatter::AddSpawnExpr(Doc& doc, const Cangjie::AST::SpawnExpr& spawnExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (spawnExpr.spawnPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "spawn ");
    }
    FuncOptions funcOptions;
    funcOptions.isLambda = true;
    funcOptions.isSpawn = true;
    if (spawnExpr.arg) {
        doc.members.emplace_back(DocType::STRING, level, "(");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(spawnExpr.arg.get(), level, funcOptions));
        doc.members.emplace_back(DocType::STRING, level, ") ");
    }
    if (spawnExpr.task) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(spawnExpr.task.get(), level, funcOptions));
    }
}
void SpawnExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto spawnExpr = As<ASTKind::SPAWN_EXPR>(node);
    AddSpawnExpr(doc, *spawnExpr, level);
}
} // namespace Cangjie::Format