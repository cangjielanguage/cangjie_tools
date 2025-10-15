// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGOTH02.h"
#include "cangjie/Basic/Match.h"
#include "common/CommonFunc.h"

/**
 * The current analysis has false positives, such as:
 * var pwd = getUserInput()
 * Future improvement direction: Based on information flow analysis, false positives can be reduced.
 * var input = getUserInput()
 * input = precess(input)
 * var pwd = input
 */

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Meta;
using namespace std;
using Json = nlohmann::json;

const std::string JSONPATH = "/config/structural_rule_G_OTH_02.json";

bool StructuralRuleGOTH02::SenInfoFilter(const std::string& key, const std::string& text)
{
    if (!jsonInfo.contains(key)) {
        Errorln(JSONPATH, " read json data failed!");
        return false;
    }

    for (const auto& item : jsonInfo[key]) {
        if (text.find(item.get<std::string>()) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool StructuralRuleGOTH02::IsWeakPasscode(const std::string& text)
{
    return SenInfoFilter("WeakPasscode", text);
}

bool StructuralRuleGOTH02::IsSenInfoName(const std::string& text)
{
    return SenInfoFilter("SenInfoName", text);
}

void StructuralRuleGOTH02::MatchPatternInAST(const VarDecl& varDecl)
{
    // if no initializer, do not check
    if (!varDecl.initializer) {
        return;
    }
    if (IsSenInfoName(varDecl.identifier)) {
        Diagnose(varDecl.begin, varDecl.end, CodeCheckDiagKind::G_OTH_02_hardcode_sensitivename_information,
            varDecl.identifier.Val());
    }
}

void StructuralRuleGOTH02::MatchPatternInAST(const VarWithPatternDecl& varWithPatternDecl)
{
    // if no initializer, do not check
    if (!varWithPatternDecl.initializer) {
        return;
    }
    Walker walker(varWithPatternDecl.irrefutablePattern.get(), [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarPattern& varPattern) {
                // there is no need to check initializer
                if (varPattern.varDecl && IsSenInfoName(varPattern.varDecl->identifier)) {
                    Diagnose(varPattern.varDecl->begin, varPattern.varDecl->end,
                        CodeCheckDiagKind::G_OTH_02_hardcode_sensitivename_information,
                        varPattern.varDecl->identifier.Val());
                }
                return VisitAction::WALK_CHILDREN;
            },
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGOTH02::FindSenInfoInAST(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl& varDecl) {
                MatchPatternInAST(varDecl);
                return VisitAction::WALK_CHILDREN;
            },
            // VarWithPatternDecl include:
            //  let (passwd,password) = ("Huawei@123","Huawei@123")
            //  let _ = "Huawei@123"
            [this](const VarWithPatternDecl& varWithPatternDecl) {
                MatchPatternInAST(varWithPatternDecl);
                return VisitAction::WALK_CHILDREN;
            },
            //  LitConstExpr is const literal
            [this](const LitConstExpr& litConstExpr) {
                if (IsWeakPasscode(litConstExpr.ToString())) {
                    Diagnose(litConstExpr.begin, litConstExpr.end,
                        CodeCheckDiagKind::G_OTH_02_hardcode_weakpasscode_information, litConstExpr.ToString());
                }
                return VisitAction::WALK_CHILDREN;
            },
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGOTH02::DealWithMatchResultfromRegex(Ptr<Node> node)
{
    if (!jsonInfo.contains("RegexSet")) {
        Errorln(JSONPATH, " read json data failed!");
        return;
    }

    std::map<int, std::regex> reg;
    const auto& regexSet = jsonInfo["RegexSet"];
    for (size_t index = 0; index < regexSet.size(); ++index) {
#ifndef CANGJIE_ENABLE_GCOV
        try {
#endif
            (void)reg.emplace(index, std::regex(regexSet[index].get<std::string>()));
#ifndef CANGJIE_ENABLE_GCOV
        } catch (const std::regex_error& e) {
            continue;
        }
#endif
    }
    std::vector<ResultInfo> result = RegexRule::InitRegexInfo(node, reg);
    auto iter = result.begin();
    auto end = result.end();
    for (; iter != end; ++iter) {
        Diagnose(iter->filepath, iter->line, iter->column, iter->endLine, iter->endColumn,
            CodeCheckDiagKind::G_OTH_02_hardcode_sensitive_information, iter->result);
    }
    RegexRule::resultVector.clear();
}

void StructuralRuleGOTH02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;

    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    // check password on ast
    FindSenInfoInAST(node);

    DealWithMatchResultfromRegex(node);
}
} // namespace Cangjie::CodeCheck