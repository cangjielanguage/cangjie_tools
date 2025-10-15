// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DATAFLOW_RULE_G_FIO_01_CHECK_H
#define DATAFLOW_RULE_G_FIO_01_CHECK_H

#include <set>
#include <string>
#include <utility>
#include "../DataflowRule.h"
#include "cangjie/CHIR/Analysis/Engine.h"
#include "cangjie/CHIR/Analysis/GenKillAnalysis.h"
#include "common/CommonFunc.h"
#include "common/DiagnosticEngine.h"

class FIO01Analysis;
class FIO01Domain : public Cangjie::CHIR::GenKillDomain<FIO01Domain> {
    friend class FIO01Analysis;

public:
    FIO01Domain() = delete;
    explicit FIO01Domain(size_t domainSize) : GenKillDomain(domainSize), allocateIdxMap(nullptr) {}
    FIO01Domain(size_t domainSize, std::map<const std::string, size_t>* allocateIdxMap)
        : GenKillDomain(domainSize), allocateIdxMap(allocateIdxMap)
    {
    }
    ~FIO01Domain() override = default;

    const std::map<const std::string, size_t>* GetMap() const { return allocateIdxMap; }

private:
    std::map<const std::string, size_t>* allocateIdxMap;
};

class FIO01Analysis final : public Cangjie::CHIR::GenKillAnalysis<FIO01Domain> {
public:
    FIO01Analysis() = delete;
    explicit FIO01Analysis(Cangjie::CHIR::Func* func, const std::vector<std::string>& files = {});
    ~FIO01Analysis() final {}

    void InitializeFuncEntryState(FIO01Domain& state) override;
    void PropagateExpressionEffect(FIO01Domain& state, const Cangjie::CHIR::Expression* expression) override;
    std::optional<Cangjie::CHIR::Block*> PropagateTerminatorEffect(
        FIO01Domain& state, const Cangjie::CHIR::Terminator* terminator) override;
    FIO01Domain Bottom() override;

private:
    std::vector<std::string> preFiles{};
    std::map<const std::string, size_t> allocateIdxMap;
};

namespace Cangjie::CodeCheck {
class DataflowRuleGFIO01Check : public DataflowRule {
public:
    DataflowRuleGFIO01Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleGFIO01Check(CodeCheckDiagnosticEngine* diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleGFIO01Check() override = default;

protected:
    std::vector<std::string> CheckDefaultParamFunc(CHIR::Func* func);
    void CheckNormalFunc(CHIR::Func* func, const std::vector<std::string>& files,
        std::map<std::string, std::pair<Cangjie::Position, Cangjie::Position>>& posMap);
    void CheckBasedOnCHIR(CHIR::Package& package) override;
};
} // namespace Cangjie::CodeCheck

#endif // DATAFLOW_RULE_G_FIO_01_CHECK_H