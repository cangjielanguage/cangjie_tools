// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_TYPEALIASDECLFORMATTER_H
#define CJFMT_TYPEALIASDECLFORMATTER_H
#include "DeclFormatter.h"

namespace Cangjie::Format {
class TypeAliasDeclFormatter : public DeclFormatter {
public:
    explicit TypeAliasDeclFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DeclFormatter(astToFormatSource, options){};

    void ASTToDoc(
        Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddTypeAliasDecl(Doc& doc, const Cangjie::AST::TypeAliasDecl& typeAliasDecl, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_TYPEALIASDECLFORMATTER_H
