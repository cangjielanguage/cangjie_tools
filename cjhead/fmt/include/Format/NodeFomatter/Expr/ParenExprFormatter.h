// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_PARENEXPRFORMATTER_H
#define CJFMT_PARENEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class ParenExprFormatter : public ExprFormatter {
public:
    explicit ParenExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddParenExpr(Doc& doc, const Cangjie::AST::ParenExpr& parenExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_PARENEXPRFORMATTER_H
