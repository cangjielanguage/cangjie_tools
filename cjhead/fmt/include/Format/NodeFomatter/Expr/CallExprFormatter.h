// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_CALLEXPRFORMATTER_H
#define CJFMT_CALLEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class CallExprFormatter : public ExprFormatter {
public:
    explicit CallExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddCallExpr(Doc& doc, const Cangjie::AST::CallExpr& callExpr, int level, FuncOptions funcOptions);
    void AddBreakLineCallArgs(Doc& doc, const Cangjie::AST::CallExpr& callExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_CALLEXPRFORMATTER_H
