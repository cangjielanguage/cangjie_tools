// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Expr/LambdaExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void LambdaExprFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto lambdaExpr = As<ASTKind::LAMBDA_EXPR>(node);
    AddLambdaExpr(doc, *lambdaExpr, level);
}

void LambdaExprFormatter::AddLambdaExpr(Doc& doc, const Cangjie::AST::LambdaExpr& lambdaExpr, int level)
{
    doc.type = DocType::LAMBDA;
    doc.indent = level;

    if (lambdaExpr.TestAttr(AST::Attribute::MOCK_SUPPORTED)) {
        doc.members.emplace_back(DocType::STRING, level, "@EnsurePreparedToMock ");
    }

    AddLambdaExprOverFlowStrategy(doc, lambdaExpr, level);
    doc.members.emplace_back(DocType::STRING, level, "{");
    if (lambdaExpr.funcBody) {
        Doc blockMember(DocType::LAMBDA_BODY, level, "");
        if (lambdaExpr.begin.line != lambdaExpr.end.line) {
            blockMember.members.emplace_back(DocType::LINE, level + 1, "");
        }
        FuncOptions funcOptions;
        funcOptions.isLambda = true;
        blockMember.members.emplace_back(astToFormatSource.ASTToDoc(lambdaExpr.funcBody.get(),
            lambdaExpr.begin.line != lambdaExpr.end.line ? level + 1 : level, funcOptions));
        if (lambdaExpr.begin.line != lambdaExpr.end.line) {
            blockMember.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(blockMember);
    }
    doc.members.emplace_back(DocType::STRING, level, "}");
    if (lambdaExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

void LambdaExprFormatter::AddLambdaExprOverFlowStrategy(Doc& doc, const Cangjie::AST::LambdaExpr& lambdaExpr, int level)
{
    switch (lambdaExpr.overflowStrategy) {
        case Cangjie::OverflowStrategy::SATURATING: {
            doc.members.emplace_back(DocType::STRING, level, "@OverflowSaturating ");
            break;
        }
        case Cangjie::OverflowStrategy::THROWING: {
            doc.members.emplace_back(DocType::STRING, level, "@OverflowThrowing ");
            break;
        }
        case Cangjie::OverflowStrategy::WRAPPING: {
            doc.members.emplace_back(DocType::STRING, level, "@OverflowWrapping ");
            break;
        }
        default: {
            break;
        }
    }
}
} // namespace Cangjie::Format