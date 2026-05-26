// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_OPTIONALCHAINEXPRFORMATTER_H
#define CJFMT_OPTIONALCHAINEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class OptionalChainExprFormatter : public ExprFormatter {
public:
    explicit OptionalChainExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddOptionalChainExpr(Doc& doc, const Cangjie::AST::OptionalChainExpr& optionalChainExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_OPTIONALCHAINEXPRFORMATTER_H
