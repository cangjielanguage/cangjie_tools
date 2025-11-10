// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_REFTYPEFORMATTER_H
#define CJFMT_REFTYPEFORMATTER_H
#include "TypeFormatter.h"
namespace Cangjie::Format {
class RefTypeFormatter : public TypeFormatter {
public:
    explicit RefTypeFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : TypeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddRefType(Doc& doc, const Cangjie::AST::RefType& refType, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_REFTYPEFORMATTER_H
