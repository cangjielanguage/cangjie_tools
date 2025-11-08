// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/FuncParamFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
#include <string>
#include <exception>

namespace Cangjie::Format {
using namespace Cangjie::AST;

void FuncParamFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto param = As<ASTKind::FUNC_PARAM>(node);
    AddFuncParam(doc, *param, level, funcOptions);
}
void FuncParamFormatter::AddFuncParam(
    Doc& doc, const Cangjie::AST::FuncParam& funcParam, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::SOFTLINE, level, "");

    if (!funcParam.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, funcParam.annotations, level, false);
        doc.members.emplace_back(DocType::LINE, level, "");
    }
    astToFormatSource.AddModifier(doc, funcParam.modifiers, level);
    if (funcParam.isMemberParam) {
        doc.members.emplace_back(DocType::STRING, level, funcParam.isVar ? "var " : "let ");
    }
    if (funcOptions.patternOrEnum) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(funcParam.type.get(), level));
    } else {
        std::string id = funcParam.identifier.GetRawText() + (funcParam.isNamedParam ? "!" : "");
        if (funcParam.type) {
            doc.members.emplace_back(DocType::STRING, level, id + ": ");
            doc.members.emplace_back(astToFormatSource.ASTToDoc(funcParam.type.get(), level));
        } else {
            doc.members.emplace_back(DocType::STRING, level, id);
        }
    }
    
    if (funcParam.assignment) {
        try {
            doc.members.emplace_back(DocType::STRING, level, " = ");
            doc.members.emplace_back(astToFormatSource.ASTToDoc(funcParam.assignment.get(), level));
        } catch (const std::exception& e) {
            if (funcParam.assignment->desugarExpr) {
                doc.members.emplace_back(astToFormatSource.ASTToDoc(funcParam.assignment->desugarExpr.get(), level));
            }
        }
    }
}
} // namespace Cangjie::Format