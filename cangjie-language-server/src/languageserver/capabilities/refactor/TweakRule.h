// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_TWEAKRULE_H
#define CANGJIE_LSP_TWEAKRULE_H

#include "Tweak.h"

namespace ark {
// base class of Tweak Rule
class TweakRule {
public:
    enum class TweakError {
        TWEAK_FAIL = 0,
        ERROR_AST
    };

    virtual ~TweakRule() = default;

    // rule check
    virtual bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const = 0;

    static bool CommonCheck(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions);
};

// Rule Engine: Check all rules
class TweakRuleEngine {
public:
    void AddRule(std::unique_ptr<TweakRule> rule)
    {
        rules.push_back(std::move(rule));
    }

    bool CheckRules(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions);

private:
    std::vector<std::unique_ptr<TweakRule>> rules;
};
} // namespace ark

#endif // CANGJIE_LSP_TWEAKRULE_H
