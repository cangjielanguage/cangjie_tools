// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_VARPATTERNFORMATTER_H
#define CJFMT_VARPATTERNFORMATTER_H
#include "PatternFormatter.h"
namespace Cangjie::Format {
class VarPatternFormatter : public PatternFormatter {
public:
    explicit VarPatternFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : PatternFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddVarPattern(Doc& doc, const Cangjie::AST::VarPattern& varPattern, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_VARPATTERNFORMATTER_H
