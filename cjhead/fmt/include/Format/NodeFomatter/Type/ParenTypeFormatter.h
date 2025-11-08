// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_PARENTYPEFORMATTER_H
#define CJFMT_PARENTYPEFORMATTER_H
#include "TypeFormatter.h"
namespace Cangjie::Format {
class ParenTypeFormatter : public TypeFormatter {
public:
    explicit ParenTypeFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : TypeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddParenType(Doc& doc, const Cangjie::AST::ParenType& parenType, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_PARENTYPEFORMATTER_H
