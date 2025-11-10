// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_SUBSCRIPTEXPRFORMATTER_H
#define CJFMT_SUBSCRIPTEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class SubscriptExprFormatter : public ExprFormatter {
public:
    explicit SubscriptExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddSubscriptExpr(
        Doc& doc, const Cangjie::AST::SubscriptExpr& subscriptExpr, int level, FuncOptions& funcOptions);
};
} // namespace Cangjie::Format
#endif // CJFMT_SUBSCRIPTEXPRFORMATTER_H
