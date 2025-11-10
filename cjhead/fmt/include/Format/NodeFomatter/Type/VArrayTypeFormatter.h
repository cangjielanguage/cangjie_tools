// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_VARRAYTYPEFORMATTER_H
#define CJFMT_VARRAYTYPEFORMATTER_H
#include "TypeFormatter.h"
namespace Cangjie::Format {
class VArrayTypeFormatter : public TypeFormatter {
public:
    explicit VArrayTypeFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : TypeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddVArrayType(Doc& doc, const Cangjie::AST::VArrayType& vArrayType, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_VARRAYTYPEFORMATTER_H
