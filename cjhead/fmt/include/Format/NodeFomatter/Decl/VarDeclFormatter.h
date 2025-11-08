// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_VARDECLFORMATTER_H
#define CJFMT_VARDECLFORMATTER_H
#include "DeclFormatter.h"

namespace Cangjie::Format {
class VarDeclFormatter : public DeclFormatter {
public:
    explicit VarDeclFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DeclFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddVarDecl(Doc& doc, const Cangjie::AST::VarDecl& varDecl, int level, FuncOptions funcOptions);
};
} // namespace Cangjie::Format
#endif // CJFMT_VARDECLFORMATTER_H
