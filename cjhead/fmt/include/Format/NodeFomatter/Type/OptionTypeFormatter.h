// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_OPTIONTYPEFORMATTER_H
#define CJFMT_OPTIONTYPEFORMATTER_H
#include "TypeFormatter.h"
namespace Cangjie::Format {
class OptionTypeFormatter : public TypeFormatter {
public:
    explicit OptionTypeFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : TypeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddOptionType(Doc& doc, const Cangjie::AST::OptionType& optionType, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_OPTIONTYPEFORMATTER_H
