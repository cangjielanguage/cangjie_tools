// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_JUMPEXPRFORMATTER_H
#define CJFMT_JUMPEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class JumpExprFormatter : public ExprFormatter {
public:
    explicit JumpExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddJumpExpr(Doc& doc, const Cangjie::AST::JumpExpr& jumpExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_JUMPEXPRFORMATTER_H
