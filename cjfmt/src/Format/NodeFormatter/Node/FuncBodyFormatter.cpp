// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/NodeFormatter/Node/FuncBodyFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "cangjie/AST/Node.h"

namespace Cangjie::Format {
using namespace Cangjie::AST;

void FuncBodyFormatter::ASTToDoc(Doc& doc, Ptr<Cangjie::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto funcBody = As<ASTKind::FUNC_BODY>(node);
    AddFuncBody(doc, *funcBody, level, funcOptions);
}

void FuncBodyFormatter::AddFuncBody(
    Doc& doc, const Cangjie::AST::FuncBody& funcBody, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    auto& generic = funcBody.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    AddFuncBodyIsLambda(doc, funcBody, level, funcOptions);

    if (funcBody.retType && !funcBody.retType->TestAttr(Attribute::COMPILER_ADD)) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(funcBody.retType.get(), level));
    }

    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }

    int changedlevel = level;
    if (funcBody.doubleArrowPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, funcBody.paramLists[0]->params.empty() ? "=>" : " =>");
        if (funcOptions.isLambda && funcBody.body) {
            if (funcBody.body->body.size() > 1) {
                changedlevel++;
                doc.members.emplace_back(DocType::LINE, changedlevel, "");
            } else if (funcBody.body->body.size() == 1) {
                doc.members.emplace_back(DocType::STRING, level, " ");
            }
        }
    }
    if (funcBody.body) {
        if (funcBody.body->leftCurlPos == INVALID_POSITION && funcBody.body->rightCurlPos == INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level, " ");
        }
    }
    funcOptions.patternOrEnum = funcBody.doubleArrowPos != INVALID_POSITION;
    doc.members.emplace_back(astToFormatSource.ASTToDoc(funcBody.body.get(), changedlevel, funcOptions));
}

void FuncBodyFormatter::AddFuncBodyIsLambda(
    Doc& doc, const Cangjie::AST::FuncBody& funcBody, int level, FuncOptions funcOptions)
{
    if (funcOptions.isLambda) {
        auto paramList = funcBody.paramLists[0].get();
        Doc params(DocType::GROUP, level, "");
        params.layout = LayoutPolicy::LAMBDA_HEADER;
        for (auto& it : paramList->params) {
            auto param = astToFormatSource.ASTToDoc(it.get(), level, funcOptions);
            // This list owns the breaks between parameters. A parameter's
            // leading soft line would duplicate the comma separator's break
            // when the lambda body requires the compatibility layout.
            if (!param.members.empty() && param.members.front().type == DocType::SOFTLINE) {
                param.members.erase(param.members.begin());
            }
            params.members.emplace_back(std::move(param));
            if (it != paramList->params.back()) {
                params.members.emplace_back(DocType::STRING, level, ",");
                Doc separator(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
                separator.breakKind = BreakKind::LIST;
                params.members.emplace_back(std::move(separator));
            }
        }
        doc.members.emplace_back(std::move(params));
    } else {
        for (auto& it : funcBody.paramLists) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(it.get(), level, funcOptions));
        }
    }
}
} // namespace Cangjie::Format
