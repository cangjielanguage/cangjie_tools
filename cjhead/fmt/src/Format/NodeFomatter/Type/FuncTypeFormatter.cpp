// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Type/FuncTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;
void Cangjie::Format::FuncTypeFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::FUNC_TYPE>(node);
    AddFuncType(doc, *type, level);
}

void FuncTypeFormatter::AddFuncType(Doc& doc, const Cangjie::AST::FuncType& funcType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!funcType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, funcType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    if (funcType.isC) {
        doc.members.emplace_back(DocType::STRING, level, "CFunc<");
    }
    doc.members.emplace_back(DocType::STRING, level, "(");
    if (!funcType.paramTypes.empty()) {
        Doc group(DocType::GROUP, level, "");
        for (auto& paramType : funcType.paramTypes) {
            group.members.emplace_back(astToFormatSource.ASTToDoc(paramType.get(), level + 1));
            if (paramType != funcType.paramTypes.back()) {
                group.members.emplace_back(DocType::STRING, level + 1, ",");
                group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            }
        }
        doc.members.emplace_back(group);
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
    doc.members.emplace_back(DocType::STRING, level, " -> ");
    if (funcType.retType) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(funcType.retType.get(), level));
    }
    if (funcType.isC) {
        doc.members.emplace_back(DocType::STRING, level, ">");
    }
}
} // namespace Cangjie::Format
