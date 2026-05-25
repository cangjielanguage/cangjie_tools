// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/CallExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void CallExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto expr = As<ASTKind::CALL_EXPR>(node);
    AddCallExpr(doc, *expr, level, funcOptions);
}

void CallExprFormatter::AddCallExpr(
    Doc& doc, const Cangjie::AST::CallExpr& callExpr, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    Doc group(DocType::GROUP, level, "");
    if (callExpr.baseFunc) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(callExpr.baseFunc.get(), level, funcOptions));
    }
    if (callExpr.baseFunc->astKind == ASTKind::LAMBDA_EXPR && callExpr.args.empty()) {
        group.members.emplace_back(DocType::STRING, level, "()");
        doc.members.emplace_back(group);
        return;
    }
    group.members.emplace_back(DocType::STRING, level, "(");
    doc.members.emplace_back(group);
    if (astToFormatSource.IsMultipleLineCallExpr(callExpr) || astToFormatSource.IsMultipleLineArg(callExpr.args)) {
        AddBreakLineCallArgs(doc, callExpr, level);
        return;
    }
    Doc args(DocType::ARGS, level, "");
    args.members.emplace_back(DocType::SOFTLINE, level + 1, "");
    if (!callExpr.args.empty()) {
        for (auto& n : callExpr.args) {
            args.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level));
            if (n->commaPos != INVALID_POSITION) {
                args.members.emplace_back(DocType::STRING, level, ",");
            }
            if (n != callExpr.args.back()) {
                args.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            }
        }
    }
    doc.members.emplace_back(args);
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (callExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
void CallExprFormatter::AddBreakLineCallArgs(Doc& doc, const Cangjie::AST::CallExpr& callExpr, int level)
{
    for (auto it = callExpr.args.begin(); it != callExpr.args.end(); ++it) {
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(it->get(), level + 1));
        if ((*it)->commaPos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level + 1, ",");
        }
        if (*it == callExpr.args.back()) {
            break;
        }
    }
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (callExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Cangjie::Format
