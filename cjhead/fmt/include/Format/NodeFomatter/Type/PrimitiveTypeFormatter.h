// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_PRIMITIVETYPEFORMATTER_H
#define CJFMT_PRIMITIVETYPEFORMATTER_H
#include "TypeFormatter.h"
namespace Cangjie::Format {
class PrimitiveTypeFormatter : public TypeFormatter {
public:
    explicit PrimitiveTypeFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : TypeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddPrimitiveType(Doc& doc, const Cangjie::AST::PrimitiveType& primitiveType, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_PRIMITIVETYPEFORMATTER_H
