// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_GENERICPARAMDECLFORMATTER_H
#define CJFMT_GENERICPARAMDECLFORMATTER_H
#include "DeclFormatter.h"

namespace Cangjie::Format {
class GenericParamDeclFormatter : public DeclFormatter {
public:
    explicit GenericParamDeclFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DeclFormatter(astToFormatSource, options){};

    void ASTToDoc(
        Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddGenericParamDecl(Doc& doc, const Cangjie::AST::GenericParamDecl& genericParamDecl, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_GENERICPARAMDECLFORMATTER_H
