// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_LITCONSTEXPRFORMATTER_H
#define CJFMT_LITCONSTEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class LitConstExprFormatter : public ExprFormatter {
public:
    explicit LitConstExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    std::string CleanString(const std::string& input);
    void AddLitConstExpr(Doc& doc, const Cangjie::AST::LitConstExpr& litConstExpr, int level);
    void MultiLineInterpolationExprProcessor(Doc& doc, Ptr<Cangjie::AST::Expr> expr, int level);
    void AddSiPartExpr(Doc& doc, Ptr<Cangjie::AST::Expr> expr, std::string& strValue, int level, bool isSingleLine);
    void AddSiPartExprs(Doc& doc, const Cangjie::AST::LitConstExpr& litConstExpr, std::string& strValue, int level,
        bool isSingleLine = false);
    void AddMultiLineRaw(Doc& doc, const Cangjie::AST::LitConstExpr& litConstExpr,
        const std::string& quote, std::string& strValue, int level);
    void AddMultiLine(Doc& doc, const Cangjie::AST::LitConstExpr& litConstExpr,
        const std::string& quote, std::string& strValue, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_LITCONSTEXPRFORMATTER_H
