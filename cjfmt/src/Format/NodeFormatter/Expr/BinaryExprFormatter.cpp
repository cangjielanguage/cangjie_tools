// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Expr/BinaryExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

namespace {
std::vector<const BinaryExpr*> CollectLeftAssociatedChain(const BinaryExpr& binaryExpr)
{
    // Follow only an existing left-associated chain of the same operator. Parentheses, precedence and
    // right-associated subexpressions remain intact; this changes layout without regrouping the AST.
    std::vector<const BinaryExpr*> chain;
    const auto* current = &binaryExpr;
    while (current != nullptr) {
        chain.emplace_back(current);
        auto left = As<ASTKind::BINARY_EXPR>(current->leftExpr.get());
        if (!left || left->op != binaryExpr.op || left->hasSemi) {
            current = nullptr;
        } else {
            current = left;
        }
    }
    return chain;
}
} // namespace

void BinaryExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto binaryExpr = As<ASTKind::BINARY_EXPR>(node);
    AddBinaryExpr(doc, *binaryExpr, level);
}

void BinaryExprFormatter::AddBinaryExpr(Doc& doc, const Cangjie::AST::BinaryExpr& binaryExpr, int level)
{
    doc.type = DocType::FILL;
    doc.indent = level;
    doc.layout = LayoutPolicy::FILL;

    auto chain = CollectLeftAssociatedChain(binaryExpr);
    Ptr<Expr> operand = chain.back()->leftExpr.get();
    for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
        const auto& operation = **it;
        Doc chunk(DocType::CONCAT, level, "");
        chunk.members.emplace_back(astToFormatSource.ASTToDoc(operand, level));
        chunk.members.emplace_back(DocType::STRING, level, " ");
        chunk.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(operation.op)]);
        doc.members.emplace_back(std::move(chunk));
        Doc separator(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        // Logical/coalescing breaks are preferred over ordinary arithmetic
        // breaks when a line has several legal candidates.
        switch (operation.op) {
            case TokenKind::COALESCING:
            case TokenKind::OR:
            case TokenKind::AND:
                separator.breakKind = BreakKind::LOGICAL;
                break;
            default:
                separator.breakKind = BreakKind::BINARY;
                break;
        }
        doc.members.emplace_back(std::move(separator));
        operand = operation.rightExpr.get();
    }
    Doc last(DocType::CONCAT, level, "");
    last.members.emplace_back(astToFormatSource.ASTToDoc(operand, level));
    if (binaryExpr.hasSemi) {
        last.members.emplace_back(DocType::STRING, level, ";");
    }
    doc.members.emplace_back(std::move(last));
}
} // namespace Cangjie::Format
