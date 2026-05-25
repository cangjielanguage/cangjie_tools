// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_INTERFACEDECLFORMATTER_H
#define CJFMT_INTERFACEDECLFORMATTER_H
#include "DeclFormatter.h"

namespace Cangjie::Format {
class InterfaceDeclFormatter : public DeclFormatter {
public:
    explicit InterfaceDeclFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DeclFormatter(astToFormatSource, options){};

    void ASTToDoc(
        Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddInterfaceDecl(Doc& doc, const Cangjie::AST::InterfaceDecl& interfaceDecl, int level);
    void AddInterfaceInheritedTypes(Doc& doc, const Cangjie::AST::InterfaceDecl& interfaceDecl, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_INTERFACEDECLFORMATTER_H
