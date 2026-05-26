// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_MEMBERACCESSFORMATTER_H
#define CJFMT_MEMBERACCESSFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class MemberAccessFormatter : public ExprFormatter {
public:
    explicit MemberAccessFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddMemberAccess(Doc& doc, const Cangjie::AST::MemberAccess& memberAccess, int level, FuncOptions funcOptions);
    static bool IsHexadecimal(const Cangjie::AST::MemberAccess& memberAccess);
};
} // namespace Cangjie::Format
#endif // CJFMT_MEMBERACCESSFORMATTER_H
