// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_PRIMARYCTORDECLFORMATTER_H
#define CJFMT_PRIMARYCTORDECLFORMATTER_H
#include "DeclFormatter.h"

namespace Cangjie::Format {
class PrimaryCtorDeclFormatter : public DeclFormatter {
public:
    explicit PrimaryCtorDeclFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : DeclFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddPrimaryCtorDecl(
        Doc& doc, const Cangjie::AST::PrimaryCtorDecl& primaryCtorDecl, int level, FuncOptions funcOptions);
};
} // namespace Cangjie::Format
#endif // CJFMT_PRIMARYCTORDECLFORMATTER_H
