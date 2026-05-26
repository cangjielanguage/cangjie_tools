// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_WILDCARDPATTERNFORMATTER_H
#define CJFMT_WILDCARDPATTERNFORMATTER_H
#include "PatternFormatter.h"
namespace Cangjie::Format {
class WildcardPatternFormatter : public PatternFormatter {
public:
    explicit WildcardPatternFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : PatternFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node>, int level, FuncOptions&) override;

private:
    void AddEnumPattern(Doc& doc, const Cangjie::AST::EnumPattern& enumPattern, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_WILDCARDPATTERNFORMATTER_H
