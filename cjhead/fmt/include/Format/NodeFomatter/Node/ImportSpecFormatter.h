// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_IMPORTSPECFORMATTER_H
#define CJFMT_IMPORTSPECFORMATTER_H
#include "Format/NodeFomatter/NodeFormatter.h"

namespace Cangjie::Format {
class ImportSpecFormatter : public NodeFormatter {
public:
    ImportSpecFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : NodeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&) override;

private:
    void AddImportSpec(Doc& doc, const Cangjie::AST::ImportSpec& importSpec, int level);
    void AddImportContent(Doc& doc, const Cangjie::AST::ImportContent& content, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_IMPORTSPECFORMATTER_H
