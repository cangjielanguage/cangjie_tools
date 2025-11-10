// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_NODEFORMATTER_H
#define CJFMT_NODEFORMATTER_H

#include "Format/Doc.h"
#include "cangjie/AST/ASTContext.h"
#include "cangjie/AST/Node.h"
#include "cangjie/Utils/Utils.h"

namespace Cangjie::Format {
class ASTToFormatSource;
class NodeFormatter {
public:
    explicit NodeFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : astToFormatSource(astToFormatSource), options(options){};
    virtual ~NodeFormatter() = default;
    virtual void ASTToDoc(
        Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) = 0;

protected:
    FormattingOptions& options;
    ASTToFormatSource& astToFormatSource;
};
} // namespace Cangjie::Format
#endif // CJFMT_NODEFORMATTER_H