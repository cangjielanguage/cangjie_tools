// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_MACROEXPANDEXPRFORMATTER_H
#define CJFMT_MACROEXPANDEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class MacroExpandExprFormatter : public ExprFormatter {
public:
    explicit MacroExpandExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddMacroExpandExpr(Doc& doc, const Cangjie::AST::MacroExpandExpr& macroExpandExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_MACROEXPANDEXPRFORMATTER_H
