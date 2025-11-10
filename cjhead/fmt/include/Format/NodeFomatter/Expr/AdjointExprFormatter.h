// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_ADJOINTEXPRFORMATTER_H
#define CJFMT_ADJOINTEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class AdjointExprFormatter : public ExprFormatter {
public:
    explicit AdjointExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddAdjointExpr(Doc& doc, const Cangjie::AST::AdjointExpr& adjointExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_ADJOINTEXPRFORMATTER_H
