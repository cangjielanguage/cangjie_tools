// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_BLOCKFORMATTER_H
#define CJFMT_BLOCKFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class BlockFormatter : public ExprFormatter {
public:
    explicit BlockFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddBlock(Doc& doc, const Cangjie::AST::Block& block, int level, FuncOptions funcOptions);
    void AddBlockIsLambda(Doc& doc, const Cangjie::AST::Block& block, int level);
    void AddSameLineCurl(Doc& doc, const Cangjie::AST::Block& block, int level);
    void AddDiffLineCurl(Doc& doc, const Cangjie::AST::Block& block, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_BLOCKFORMATTER_H
