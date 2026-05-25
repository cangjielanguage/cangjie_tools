// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_DOWHILEEXPRFORMATTER_H
#define CJFMT_DOWHILEEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class DoWhileExprFormatter : public ExprFormatter {
public:
    explicit DoWhileExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddDoWhileExpr(Doc& doc, const Cangjie::AST::DoWhileExpr& doWhileExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_DOWHILEEXPRFORMATTER_H
