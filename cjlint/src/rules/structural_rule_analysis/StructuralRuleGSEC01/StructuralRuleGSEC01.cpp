// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGSEC01.h"
#include "common/CommonFunc.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

const std::string JSONPATH = "/config/structural_rule_G_SEC_01.json";

void StructuralRuleGSEC01::RecordLocation(Ptr<FuncDecl> funcDecl)
{
    if (!jsonInfo.contains("CheckingFunction")) {
        Errorln(JSONPATH, " read json data failed!");
        return;
    }

    for (const auto& item : jsonInfo["CheckingFunction"]) {
        std::regex reg = std::regex(item.get<std::string>(), std::regex_constants::icase);
        if (std::regex_match(funcDecl->identifier.Val(), reg)) {
            Diagnose(funcDecl->identifier.Begin(), funcDecl->identifier.End(),
                CodeCheckDiagKind::G_SEC_01_open_check_function_information, funcDecl->identifier.Val());
        }
    }
}

bool StructuralRuleGSEC01::IsExtendClass(const ClassDecl& classDecl) const
{
    for (auto i : classDecl.modifiers) {
        if (i.modifier == TokenKind::OPEN) {
            return true;
        }
    }
    if (!classDecl.body) {
        return false;
    }
    for (auto& classBody : classDecl.body->decls) {
        for (auto i : classBody->modifiers) {
            if (i.modifier == TokenKind::OPEN) {
                return true;
            }
        }
    }
    return false;
}

void StructuralRuleGSEC01::ClassDeclHandlerDetail(const OwnedPtr<Decl>& classBody, const bool isExtend)
{
    if (classBody.get() != nullptr && classBody.get()->astKind == ASTKind::FUNC_DECL) {
        auto funcDecl = StaticAs<ASTKind::FUNC_DECL>(classBody.get());
        for (auto item : funcDecl->modifiers) {
            if ((item.modifier == TokenKind::OPEN) || (item.modifier == TokenKind::STATIC && isExtend)) {
                RecordLocation(funcDecl);
            }
        }
    }
}

void StructuralRuleGSEC01::ClassDeclHandler(const ClassDecl& classDecl)
{
    bool isExtend = IsExtendClass(classDecl);
    if (!classDecl.body) {
        return;
    }
    for (auto& classBody : classDecl.body->decls) {
        ClassDeclHandlerDetail(classBody, isExtend);
    }
}

void StructuralRuleGSEC01::InterfaceDeclHandler(const InterfaceDecl& interfaceDecl)
{
    if (interfaceDecl.body) {
        for (auto& interfaceBoyd : interfaceDecl.body->decls) {
            if (interfaceBoyd.get() != nullptr && interfaceBoyd.get()->astKind == ASTKind::FUNC_DECL) {
                auto funcDecl = StaticAs<ASTKind::FUNC_DECL>(interfaceBoyd.get());
                RecordLocation(funcDecl);
            }
        }
    }
}

void StructuralRuleGSEC01::FindCheckingFunction(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            // Identifying Variable Declaration Statements in Programs
            [this](const ClassDecl& classDecl) {
                ClassDeclHandler(classDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const InterfaceDecl& interfaceDecl) {
                InterfaceDeclHandler(interfaceDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            // The rest of the statements are here.
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGSEC01::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;

    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    FindCheckingFunction(node);
}
} // namespace Cangjie::CodeCheck
