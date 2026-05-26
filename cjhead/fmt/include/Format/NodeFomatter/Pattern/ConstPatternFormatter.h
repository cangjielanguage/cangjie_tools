// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_CONSTPATTERNFORMATTER_H
#define CJFMT_CONSTPATTERNFORMATTER_H
#include "PatternFormatter.h"
namespace Cangjie::Format {
class ConstPatternFormatter : public PatternFormatter {
public:
    explicit ConstPatternFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : PatternFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddConstPattern(Doc& doc, const Cangjie::AST::ConstPattern& constPattern, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_CONSTPATTERNFORMATTER_H
