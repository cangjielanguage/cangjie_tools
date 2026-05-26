// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_FUNCPARAMLISTFORMATTER_H
#define CJFMT_FUNCPARAMLISTFORMATTER_H
#include "Format/NodeFomatter/NodeFormatter.h"
#include <vector>

namespace Cangjie::Format {
class FuncParamListFormatter : public NodeFormatter {
public:
    explicit FuncParamListFormatter(ASTToFormatSource& astToFormatSource, FormattingOptions& options)
        : NodeFormatter(astToFormatSource, options){};

    void ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions) override;

private:
    void AddFuncParamList(Doc& doc, const AST::FuncParamList& funcParamList, int level, FuncOptions funcOptions);
    bool IsMultipleLineMacroExpandParam(const AST::FuncParamList& funcParamList);
    bool IsMultipleLine(const int& rightParentPosLine, const std::vector<OwnedPtr<AST::FuncParam>>& params) const;
    void AddEmptyParam(Doc& doc, int level);
};
} // namespace Cangjie::Format
#endif // CJFMT_FUNCPARAMLISTFORMATTER_H
