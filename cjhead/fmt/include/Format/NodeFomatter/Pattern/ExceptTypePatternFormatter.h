// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_EXCEPTTYPEPATTERNFORMATTER_H
#define CJFMT_EXCEPTTYPEPATTERNFORMATTER_H
#include "PatternFormatter.h"
namespace Cangjie::Format {
class ExceptTypePatternFormatter : public PatternFormatter {
public:
    explicit ExceptTypePatternFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : PatternFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddExceptTypePattern(Doc& doc, const Cangjie::AST::ExceptTypePattern& exceptTypePattern, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_EXCEPTTYPEPATTERNFORMATTER_H
