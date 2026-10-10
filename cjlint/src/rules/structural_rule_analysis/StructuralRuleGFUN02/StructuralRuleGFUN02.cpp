// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGFUN02.h"
#include "cangjie/Basic/Match.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;

/*
 * The function that find the FuncExpr's params and insert them to param Map.
 *
 */
void StructuralRuleGFUN02::FuncParamFinder(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const FuncDecl& funcDecl) {
                ProcessFuncDecl(funcDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGFUN02::ProcessFuncDecl(const Cangjie::AST::FuncDecl &funcDecl)
{
    if (!funcDecl.funcBody) {
        return;
    }
    std::map<std::string, PositionPair> paramMap;
    for (auto &list : funcDecl.funcBody->paramLists) {
        for (auto &param : list->params) {
            paramMap[param->identifier] = {param->identifier.Begin(), param->identifier.End()};
        }
    }
    for (auto &list : funcDecl.funcBody->paramLists) {
        for (auto &param : list->params) {
            if (param->assignment != nullptr) {
                EraseUsedParams(param->assignment.get(), paramMap);
            }
        }
    }
    FuncParamFinderHelper(funcDecl.funcBody->body.get(), paramMap);
    if (!funcDecl.funcBody->body) {
        return;
    }
    for (auto &item : funcDecl.funcBody->body->body) {
        if (item->astKind == ASTKind::FUNC_DECL) {
            FuncParamFinder(item.get());
        }
    }
}

void StructuralRuleGFUN02::EraseItemOfMap(const std::string& id, std::map<std::string, PositionPair>& paramMap)
{
    auto iter = paramMap.find(id);
    if (iter != paramMap.end()) {
        (void)paramMap.erase(id);
    }
}
void StructuralRuleGFUN02::EraseUsedParams(Ptr<Node> node, std::map<std::string, PositionPair>& paramMap)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [&paramMap](Ptr<Node> node) -> VisitAction {
        match (*node)(
            [&paramMap](const RefExpr& refExpr) {
                EraseItemOfMap(refExpr.ref.identifier.GetRawText(), paramMap);
            },
            [&paramMap](const MacroExpandExpr& macroExpandExpr) {
                for (auto& arg : macroExpandExpr.invocation.args) {
                    if (arg.kind != TokenKind::IDENTIFIER) {
                        continue;
                    }
                    EraseItemOfMap(arg.Value(), paramMap);
                }
            });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

/*
 * The function that find the param which is used and delete it from param map.
 *
 */
void StructuralRuleGFUN02::FuncParamFinderHelper(Ptr<Node> node, std::map<std::string, PositionPair>& paramMap)
{
    if (node == nullptr) {
        return;
    }

    EraseUsedParams(node, paramMap);

    // if the paramMap is not empty that means the param which in map is not be used, throw error.
    if (!paramMap.empty()) {
        for (auto& param : paramMap) {
            if (param.first == "_") {
                continue;
            }
            Diagnose(param.second.first, param.second.second, CodeCheckDiagKind::G_FUN_02_unused_param_information,
                param.first.c_str());
        }
    }
}

void StructuralRuleGFUN02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FuncParamFinder(node);
}
} // namespace Cangjie::CodeCheck
