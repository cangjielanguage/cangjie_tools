// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <regex>
#include "StructuralRuleGNAM05.h"

using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;
using namespace CodeCheck;

/**
 * The names of immutable global variables or static variables must be in uppercase.
 * right eg:
 * MAXNUM, STRINGNAME
 * wrong eg:
 * maxNum, MaxNum
 */
static const std::string ALL_CAPITAL_LETTERS = "^[A-Z_][A-Z0-9_]*";

void StructuralRuleGNAM05::CheckGlobalOrStaticVarWithLetName(const Cangjie::AST::VarDecl& varDecl)
{
    if (!regex_match(varDecl.identifier.Val(), std::regex(ALL_CAPITAL_LETTERS))) {
        Diagnose(varDecl.begin, varDecl.end, CodeCheckDiagKind::G_NAM_05_immutable_global_variable_naming_information,
            varDecl.identifier.Val());
    }
}

void StructuralRuleGNAM05::FindGlobalOrStaticVarWithLetName(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl& varDecl) {
                if (!varDecl.isVar &&
                    (varDecl.TestAttr(AST::Attribute::GLOBAL) || varDecl.TestAttr(AST::Attribute::STATIC))) {
                    CheckGlobalOrStaticVarWithLetName(varDecl);
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGNAM05::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindGlobalOrStaticVarWithLetName(node);
}
