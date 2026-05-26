// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_TUPLEPATTERNFORMATTER_H
#define CJFMT_TUPLEPATTERNFORMATTER_H
#include "PatternFormatter.h"
namespace Cangjie::Format {
class TuplePatternFormatter : public PatternFormatter {
public:
    explicit TuplePatternFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : PatternFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddTuplePattern(Doc& doc, const Cangjie::AST::TuplePattern& tuplePattern, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_TUPLEPATTERNFORMATTER_H
