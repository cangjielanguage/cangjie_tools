// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFomatter/Expr/MemberAccessFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"
#include <string>

namespace Cangjie::Format {
using namespace Cangjie::AST;

void MemberAccessFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto expr = As<ASTKind::MEMBER_ACCESS>(node);
    if (funcOptions.isMethodChainning || !options.allowMultiLineMethodChain) {
        AddMemberAccess(doc, *expr, level, funcOptions);
        return;
    }

    if (astToFormatSource.IsMultipleMethodChain(*expr) >= options.multipleLineMethodChainLevel) {
        funcOptions.isMethodChainning = true;
    }
    AddMemberAccess(doc, *expr, level, funcOptions);
}

void MemberAccessFormatter::AddMemberAccess(
    Doc& doc, const Cangjie::AST::MemberAccess& memberAccess, int level, FuncOptions funcOptions)
{
    doc.type = DocType::MEMBER_ACCESS;
    doc.indent = level;
    doc.members.emplace_back(astToFormatSource.ASTToDoc(memberAccess.baseExpr.get(), level, funcOptions));
    if (funcOptions.isMethodChainning) {
        doc.members.emplace_back(DocType::LINE_DOT, level + 1, ".");
    } else {
        doc.members.emplace_back(DocType::DOT, level, ".");
    }
    if (IsHexadecimal(memberAccess)) {
        doc.members.emplace_back(DocType::LINE, level, "");
    }

    doc.members.emplace_back(DocType::STRING, level,
        memberAccess.field.IsRaw() ? memberAccess.field.GetRawText() : memberAccess.field.Val());
    if (!memberAccess.typeArguments.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "<");
        for (auto& n : memberAccess.typeArguments) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level + 1));
            if (n != memberAccess.typeArguments.back()) {
                doc.members.emplace_back(DocType::STRING, level + 1, ", ");
            }
    }
        doc.members.emplace_back(DocType::STRING, level, ">");
    }
    if (memberAccess.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

bool MemberAccessFormatter::IsHexadecimal(const Cangjie::AST::MemberAccess& memberAccess)
{
    if (memberAccess.baseExpr != nullptr && Is<LitConstExpr>(memberAccess.baseExpr.get()) &&
        StaticAs<ASTKind::LIT_CONST_EXPR>(memberAccess.baseExpr.get())->kind == LitConstKind::INTEGER) {
        auto litConstExpr = As<ASTKind::LIT_CONST_EXPR>(memberAccess.baseExpr.get());
        if (litConstExpr != nullptr && !litConstExpr->stringValue.empty()) {
            std::string str = litConstExpr->stringValue.substr(0, 2);
            return str.find("0x") != std::string::npos || str.find("0X") != std::string::npos;
        }
    }
    return false;
}
} // namespace Cangjie::Format