// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CHECKER_H
#define CHECKER_H

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include "Cjlint.h"
#include "common/ConfigContext.h"
#include "common/DiagnosticEngine.h"
#include "rules/Rule.h"

namespace Cangjie::CodeCheck {
using RulesMap = std::map<CompileStage, std::map<const std::string, std::shared_ptr<Rule>>>;
class Checker {
public:
    explicit Checker(CodeCheckDiagnosticEngine* diagEngine);
    ~Checker() = default;
    int CheckCode();

private:
    template <typename T> void RegisterRule(const std::string& name, CompileStage stage)
    {
        rulesMap[stage][name] = std::make_shared<T>(diagEngine);
    }

    static void DoAnalysis(Rule* rule, CJLintCompilerInstance* instance) { rule->DoAnalysis(instance); }

    std::string fileDir;
    std::string modulesDir;
    CodeCheckDiagnosticEngine* diagEngine;
    RulesMap rulesMap;

    void CreateRuleThreads(nlohmann::json jsonInfo, const std::unique_ptr<CJLintCompilerInstance>& instance,
        CompileStage compileStage, int& readJsonCode);
};
} // namespace Cangjie::CodeCheck

#endif // CHECKER_H