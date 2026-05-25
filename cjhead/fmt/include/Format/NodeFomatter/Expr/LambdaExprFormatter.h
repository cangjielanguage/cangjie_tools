// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_LAMBDAEXPRFORMATTER_H
#define CJFMT_LAMBDAEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class LambdaExprFormatter : public ExprFormatter {
public:
    explicit LambdaExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddLambdaExpr(Doc& doc, const Cangjie::AST::LambdaExpr& lambdaExpr, int level);
    void AddLambdaExprOverFlowStrategy(Doc& doc, const Cangjie::AST::LambdaExpr& lambdaExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_LAMBDAEXPRFORMATTER_H
