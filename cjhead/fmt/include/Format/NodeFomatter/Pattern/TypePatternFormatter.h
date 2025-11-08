// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_TYPEPATTERNFORMATTER_H
#define CJFMT_TYPEPATTERNFORMATTER_H
#include "PatternFormatter.h"
namespace Cangjie::Format {
class TypePatternFormatter : public PatternFormatter {
public:
    explicit TypePatternFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : PatternFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddTypePattern(Doc& doc, const Cangjie::AST::TypePattern& typePattern, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_TYPEPATTERNFORMATTER_H
