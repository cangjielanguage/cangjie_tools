// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_ENUMDECLFORMATTER_H
#define CJFMT_ENUMDECLFORMATTER_H
#include "DeclFormatter.h"

namespace Cangjie::Format {
class EnumDeclFormatter : public DeclFormatter {
public:
    explicit EnumDeclFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DeclFormatter(astToFormatSource, options){};

    void ASTToDoc(
        Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddEnumMembers(Doc& doc, const Cangjie::AST::EnumDecl& enumDecl, int level);
    void AddEnumBreakLineConstructors(Doc& doc, const Cangjie::AST::EnumDecl& enumDecl, int level);
    void AddEnumInheritedTypes(Doc& doc, const Cangjie::AST::EnumDecl& enumDecl, int level);
    void AddEnumDecl(Doc& doc, const Cangjie::AST::EnumDecl& enumDecl, int level);
    bool HasFirstOptionalSeparator(const AST::EnumDecl& ed);
};
} // namespace Cangjie::Format
#endif // CJFMT_ENUMDECLFORMATTER_H
