// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef RULES_CHECKER_H
#define RULES_CHECKER_H

#include <functional>
#include <vector>

#include "cjcompat/Diff/Diff.h"
#include "cjcompat/Dsl/Dsl.h"
#include "cjcompat/Logger/Logger.h"
#include "cjcompat/CheckersAndRules/CheckersList.inc"

class Checker {
public:
    Checker(std::function<bool(Dsl&, const Diff&, Logger&, const Checker&)> function, const size_t precedence,
        std::set<RuleKind> checkedRules)
        : function(function), checkedRules(checkedRules), precedence(precedence)
    {
    }
    bool Run(Dsl& dsl, const Diff& diff, Logger& logger)
    {
        return function(dsl, diff, logger, *this);
    }
    bool Checks(RuleKind kind) const
    {
        return checkedRules.find(kind) != checkedRules.end();
    }
    size_t Precedence() const
    {
        return precedence;
    }

private:
    std::set<RuleKind> checkedRules;
    size_t precedence;
    std::function<bool(Dsl&, const Diff&, Logger&, const Checker&)> function;
};

struct NodeInfo {
    Node* n1;
    Node* n2;
    const Diff* diff;
};

namespace CheckerImpl {
#define CHECKER_DECL(NAME, PRECEDENCE, RULE_KIND...)                                                                   \
    bool NAME(Dsl& dsl, const Diff& diff, Logger& logger, const Checker&);
#include "CheckersList.inc"
#undef CHECKER_DECL

bool IsCheckedAlready(std::vector<Node*>& vec, Node* n);
bool ChangeFuncParamOrder(Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& funcInfo);
void CheckFuncParams(Dsl& dsl, Logger& logger, const Checker& checker, bool& checkerResult, NodeInfo& funcInfo);
}; // namespace CheckerImpl

class AllCheckers {
public:
    bool RunAll(Dsl& dsl, const Diff& diff, Logger& logger)
    {
        bool res = true;
        /* sort checkers by precedence so that checkers with lower precedence are executed before checkers with higher
        precedence. Lower precedence means higher priority. */
        std::stable_sort(checkers.begin(), checkers.end(),
            [](const Checker& c1, const Checker& c2) { return c1.Precedence() < c2.Precedence(); });
        size_t lastPrecedence = checkers.empty() ? 0 : checkers.begin()->Precedence();
        for (auto checker : checkers) {
            if (!res && checker.Precedence() > lastPrecedence) {
                // Stop if a checker with a lower precedence (higher priority) has failed before.
                return res;
            }
            auto r = checker.Run(dsl, diff, logger);
            res = res && r;
            lastPrecedence = checker.Precedence();
        }
        return res;
    }

private:
    std::vector<Checker> checkers{
#define CHECKER_DECL(NAME, PRECEDENCE, RULE_KIND...) Checker{CheckerImpl::NAME, PRECEDENCE, {RULE_KIND}},
#include "CheckersList.inc"
#undef CHECKER_DECL
    };
};

#endif // RULES_CHECKER_H