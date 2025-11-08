// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_THISTYPEFORMATTER_H
#define CJFMT_THISTYPEFORMATTER_H
#include "TypeFormatter.h"
namespace Cangjie::Format {
class ThisTypeFormatter : public TypeFormatter {
public:
    explicit ThisTypeFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : TypeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node>, int level, FuncOptions&) override;

private:
    void AddThisType(Doc& doc, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_THISTYPEFORMATTER_H
