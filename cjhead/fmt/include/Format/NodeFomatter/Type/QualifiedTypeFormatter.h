// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_QUALIFIEDTYPEFORMATTER_H
#define CJFMT_QUALIFIEDTYPEFORMATTER_H
#include "TypeFormatter.h"
namespace Cangjie::Format {
class QualifiedTypeFormatter : public TypeFormatter {
public:
    explicit QualifiedTypeFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : TypeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddQualifiedType(Doc& doc, const Cangjie::AST::QualifiedType& qualifiedType, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_QUALIFIEDTYPEFORMATTER_H
