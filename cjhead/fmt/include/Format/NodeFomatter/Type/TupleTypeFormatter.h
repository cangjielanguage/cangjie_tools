// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_TUPLETYPEFORMATTER_H
#define CJFMT_TUPLETYPEFORMATTER_H
#include "TypeFormatter.h"
namespace Cangjie::Format {
class TupleTypeFormatter : public TypeFormatter {
public:
    explicit TupleTypeFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : TypeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddTupleType(Doc& doc, const Cangjie::AST::TupleType& tupleType, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_TUPLETYPEFORMATTER_H
