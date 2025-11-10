// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Node/FuncParamListFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
#include <vector>
namespace Cangjie::Format {
const int MIN_MUL_MEMBERS = 2;
using namespace Cangjie::AST;

void FuncParamListFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto paramList = As<ASTKind::FUNC_PARAM_LIST>(node);
    AddFuncParamList(doc, *paramList, level, funcOptions);
}

bool FuncParamListFormatter::IsMultipleLineMacroExpandParam(const AST::FuncParamList& funcParamList)
{
    if (funcParamList.params.size() > 1) {
        for (auto& n : funcParamList.params) {
            if (n->astKind == ASTKind::MACRO_EXPAND_PARAM) {
                return true;
            }
        }
    } else if (funcParamList.params.size() == 1) {
        if (funcParamList.params[0]->astKind == ASTKind::MACRO_EXPAND_PARAM) {
            auto  pMacroExpendParam = As<ASTKind::MACRO_EXPAND_PARAM>(funcParamList.params[0]);
            if (pMacroExpendParam->invocation.rightSquarePos != INVALID_POSITION &&
                pMacroExpendParam->invocation.rightSquarePos.line !=
                pMacroExpendParam->invocation.attrs.back().End().line) {
                return true;
            }
            if (pMacroExpendParam->invocation.decl &&
                pMacroExpendParam->invocation.decl->astKind == AST::ASTKind::MACRO_EXPAND_PARAM) {
                return true;
            }
        }
    }
    return false;
}

void FuncParamListFormatter::AddFuncParamList(
    Doc& doc, const AST::FuncParamList& funcParamList, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (funcParamList.params.empty()) {
        AddEmptyParam(doc, level);
        return;
    }
    doc.members.emplace_back(DocType::STRING, level, "(");
    doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");

    bool isMultipleLineMacroExpandParam = IsMultipleLineMacroExpandParam(funcParamList);
    if (isMultipleLineMacroExpandParam || IsMultipleLine(funcParamList.rightParenPos.line, funcParamList.params)) {
        astToFormatSource.AddBreakLineParam(doc, funcParamList, level, funcOptions);
        return;
    }

    for (auto& n : funcParamList.params) {
        Doc group(DocType::GROUP, level + 1, "");
        group.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level + 1, funcOptions));
        if (n != funcParamList.params.back()) {
            group.members.emplace_back(DocType::STRING, level + 1, ",");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
        if (n == funcParamList.params.back() && n->commaPos != INVALID_POSITION) {
            group.members.emplace_back(DocType::STRING, level + 1, ", ...");
        }
        doc.members.emplace_back(group);
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
}

bool FuncParamListFormatter::IsMultipleLine(const int& rightParentPosLine,
    const std::vector<OwnedPtr<AST::FuncParam>>& params) const
{
    if (params.size() < MIN_MUL_MEMBERS) {
        return false;
    }
    if (rightParentPosLine == params.back()->end.line) {
        return false;
    }
    return true;
}

void FuncParamListFormatter::AddEmptyParam(Doc& doc, int level)
{
    doc.members.emplace_back(DocType::STRING, level, "()");
}
} // namespace Cangjie::Format