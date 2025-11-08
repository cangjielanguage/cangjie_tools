// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_CLASSBODYFORMATTER_H
#define CJFMT_CLASSBODYFORMATTER_H
#include "Format/NodeFomatter/NodeFormatter.h"

namespace Cangjie::Format {
class ClassBodyFormatter : public NodeFormatter {
public:
    explicit ClassBodyFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : NodeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddClassBody(Doc& doc, const Cangjie::AST::ClassBody& classBody, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_CLASSBODYFORMATTER_H
