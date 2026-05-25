// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_SPAWNEXPRFORMATTER_H
#define CJFMT_SPAWNEXPRFORMATTER_H
#include "ExprFormatter.h"
namespace Cangjie::Format {
class SpawnExprFormatter : public ExprFormatter {
public:
    explicit SpawnExprFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : ExprFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddSpawnExpr(Doc& doc, const Cangjie::AST::SpawnExpr& spawnExpr, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_SPAWNEXPRFORMATTER_H
