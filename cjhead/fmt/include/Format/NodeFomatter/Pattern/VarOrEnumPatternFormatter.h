// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_VARORENUMPATTERNFORMATTER_H
#define CJFMT_VARORENUMPATTERNFORMATTER_H
#include "PatternFormatter.h"
namespace Cangjie::Format {
class VarOrEnumPatternFormatter : public PatternFormatter {
public:
    explicit VarOrEnumPatternFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : PatternFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddVarOrEnumPattern(Doc& doc, const Cangjie::AST::VarOrEnumPattern& varOrEnumPattern, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_VARORENUMPATTERNFORMATTER_H
