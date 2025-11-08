// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_QUOTEEXPRFORMATTER_H
#define CJFMT_QUOTEEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class QuoteExprFormatter : public ExprFormatter {
public:
    explicit QuoteExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddQuoteExpr(Doc& doc, const Cangjie::AST::QuoteExpr& quoteExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_QUOTEEXPRFORMATTER_H
