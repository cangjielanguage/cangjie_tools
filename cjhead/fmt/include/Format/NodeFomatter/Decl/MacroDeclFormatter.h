// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_MACRODECLFORMATTER_H
#define CJFMT_MACRODECLFORMATTER_H
#include "DeclFormatter.h"

namespace Cangjie::Format {
class MacroDeclFormatter : public DeclFormatter {
public:
    explicit MacroDeclFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DeclFormatter(astToFormatSource, options){};

    void ASTToDoc(
        Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddMacroDecl(Doc& doc, const Cangjie::AST::MacroDecl& macroDecl, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_MACRODECLFORMATTER_H
