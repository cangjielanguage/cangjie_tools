// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef RULES_RULE_H
#define RULES_RULE_H

#include <string>

#include "cjcompat/Diff/Diff.h"
#include "cjcompat/Dsl/Dsl.h"

enum class RuleKind : size_t {
#define RULEKIND(ID, IS_API, IS_ABI, DESCRIPTION) ID,
#include "cjcompat/CheckersAndRules/RulesList.inc"

#undef RULEKIND
};

class Rule {
public:
    Rule(RuleKind kind, bool isAPI, bool isABI, std::string description)
        : kind(kind), isAPI(isAPI), isABI(isABI), description(description)
    {
        CJC_ASSERT(isAPI || isABI);
    }

    bool IsAPI() const
    {
        return isAPI;
    }

    bool IsABI() const
    {
        return isABI;
    }

    std::string GetDescription() const
    {
        return description;
    }

private:
    RuleKind kind;
    bool isAPI;
    bool isABI;
    std::string description;
};

namespace Rules {
const static std::vector<Rule> RULE_KIND_2_RULE{
#define RULEKIND(ID, IS_API, IS_ABI, DESCRIPTION) {Rule{RuleKind::ID, IS_API, IS_ABI, DESCRIPTION}},
#include "RulesList.inc"
#undef RULEKIND
};

const static Rule& GetRule(RuleKind kind)
{
    return RULE_KIND_2_RULE[static_cast<size_t>(kind)];
}
} // namespace Rules

#endif // RULES_RULE_H