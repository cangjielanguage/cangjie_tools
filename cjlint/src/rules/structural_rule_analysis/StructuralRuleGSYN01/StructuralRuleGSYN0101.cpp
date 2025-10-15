// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGSYN0101.h"
#include "common/CommonFunc.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace AST;
using namespace Meta;

constexpr const char* JSONPATH = "/config/structural_rule_G_SYN_01.json";

void StructuralRuleGSYN0101::InitDisabledSyntaxCheckMap()
{
    disabledSyntaxCheckMap["PrimitiveType"] = handlePrimitiveType;
}

// Collect keywords for disabling syntax
void StructuralRuleGSYN0101::CollectDisabledSyntax()
{
    std::unordered_set<std::string> disabledSyntaxKeywords;
    if (!jsonInfo.contains("SyntaxKeyword")) {
        Errorln(JSONPATH, " read json data failed!");
        return;
    }

    // Ensure consistency with the parameter requirements for the JsonArrayStringValueGet() interface
    for (const auto& item : jsonInfo["SyntaxKeyword"]) {
        std::string keyWord = item.get<std::string>();
        disabledSyntaxKeywords.insert(keyWord);
    }
    for (auto& item : disabledSyntaxKeywords) {
        if (disabledSyntaxCheckMap.count(item) != 0) {
            syntaxList.insert(item);
        }
    }
}

void StructuralRuleGSYN0101::FindSyntax(Ptr<Cangjie::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        // Check whether the syntax is violated based on the check on some kind of AST nodes.
        for (auto& syntaxName : syntaxList) {
            disabledSyntaxCheckMap[syntaxName](*node);
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGSYN0101::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    InitDisabledSyntaxCheckMap();
    CollectDisabledSyntax();
    FindSyntax(node);
}
} // namespace Cangjie::CodeCheck