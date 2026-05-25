// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_ISEXPRFORMATTER_H
#define CJFMT_ISEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class IsExprFormatter : public ExprFormatter {
public:
    explicit IsExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddIsExpr(Doc& doc, const Cangjie::AST::IsExpr& isExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_ISEXPRFORMATTER_H
