// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_FUNCARGFORMATTER_H
#define CJFMT_FUNCARGFORMATTER_H
#include "Format/NodeFomatter/NodeFormatter.h"

namespace Cangjie::Format {
class FuncArgFormatter : public NodeFormatter {
public:
    explicit FuncArgFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : NodeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddFuncArg(Doc& doc, const Cangjie::AST::FuncArg& funcArg, int level, FuncOptions& funcOptions);
};
} // namespace Cangjie::Format
#endif // CJFMT_FUNCARGFORMATTER_H
