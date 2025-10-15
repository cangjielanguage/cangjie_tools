// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DATAFLOW_RULE_G_CHK_03_CHECK_H
#define DATAFLOW_RULE_G_CHK_03_CHECK_H

#include <set>
#include <string>
#include <utility>
#include "../DataflowRule.h"
#include "cangjie/CHIR/Analysis/Engine.h"
#include "cangjie/CHIR/Analysis/GenKillAnalysis.h"
#include "common/CommonFunc.h"
#include "common/DiagnosticEngine.h"

namespace Cangjie::CodeCheck {
class DataflowRuleGCHK03Check : public DataflowRule {
public:
    DataflowRuleGCHK03Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleGCHK03Check(CodeCheckDiagnosticEngine* diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleGCHK03Check() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package& package) override;
    void CheckGlobalFunc(CHIR::Func* func);
    void IsPathCanonicalized(CHIR::Func* func);
    void IsPathVerified(CHIR::Func* func);

private:
    nlohmann::json jsonInfo;
};
// This part is used to check "The file path constructed by external data must be canonicalized before being verified."
class CHK03CanonicalizeAnalysis;
class CHK03CanonicalizeDomain : public Cangjie::CHIR::GenKillDomain<CHK03CanonicalizeDomain> {
    friend class CHK03CanonicalizeAnalysis;

public:
    CHK03CanonicalizeDomain() = delete;
    explicit CHK03CanonicalizeDomain(size_t domainSize) : GenKillDomain(domainSize), allocateIdxMap(nullptr) {}
    CHK03CanonicalizeDomain(size_t domainSize, std::map<const std::string, size_t>* allocateIdxMap)
        : GenKillDomain(domainSize), allocateIdxMap(allocateIdxMap)
    {
    }
    ~CHK03CanonicalizeDomain() override = default;

    const std::map<const std::string, size_t>* GetMap() const { return allocateIdxMap; }

private:
    std::map<const std::string, size_t>* allocateIdxMap;
};

class CHK03CanonicalizeAnalysis final : public Cangjie::CHIR::GenKillAnalysis<CHK03CanonicalizeDomain> {
public:
    CHK03CanonicalizeAnalysis() = delete;
    explicit CHK03CanonicalizeAnalysis(Cangjie::CHIR::Func* func, nlohmann::json jsonInfo);
    ~CHK03CanonicalizeAnalysis() final {}

    void InitializeFuncEntryState(CHK03CanonicalizeDomain& state) override;
    void PropagateExpressionEffect(
        CHK03CanonicalizeDomain& state, const Cangjie::CHIR::Expression* expression) override;
    std::optional<Cangjie::CHIR::Block*> PropagateTerminatorEffect(
        CHK03CanonicalizeDomain& state, const Cangjie::CHIR::Terminator* terminator) override;
    CHK03CanonicalizeDomain Bottom() override;

private:
    nlohmann::json jsonInfo;
    std::vector<std::string> preFiles{};
    std::map<const std::string, size_t> allocateIdxMap;
};

// This part is used to check "The file path constructed by external data must be verified before being used."
class CHK03VerifyAnalysis;
class CHK03VerifyDomain : public Cangjie::CHIR::GenKillDomain<CHK03VerifyDomain> {
    friend class CHK03VerifyAnalysis;

public:
    CHK03VerifyDomain() = delete;
    explicit CHK03VerifyDomain(size_t domainSize) : GenKillDomain(domainSize), allocateIdxMap(nullptr) {}
    CHK03VerifyDomain(size_t domainSize, std::map<const std::string, size_t>* allocateIdxMap)
        : GenKillDomain(domainSize), allocateIdxMap(allocateIdxMap)
    {
    }
    ~CHK03VerifyDomain() override = default;

    const std::map<const std::string, size_t>* GetMap() const { return allocateIdxMap; }

private:
    std::map<const std::string, size_t>* allocateIdxMap;
};

class CHK03VerifyAnalysis final : public Cangjie::CHIR::GenKillAnalysis<CHK03VerifyDomain> {
public:
    CHK03VerifyAnalysis() = delete;
    explicit CHK03VerifyAnalysis(Cangjie::CHIR::Func* func, nlohmann::json jsonInfo);
    ~CHK03VerifyAnalysis() final {}

    void InitializeFuncEntryState(CHK03VerifyDomain& state) override;
    void PropagateExpressionEffect(CHK03VerifyDomain& state, const Cangjie::CHIR::Expression* expression) override;
    std::optional<Cangjie::CHIR::Block*> PropagateTerminatorEffect(
        CHK03VerifyDomain& state, const Cangjie::CHIR::Terminator* terminator) override;
    CHK03VerifyDomain Bottom() override;

private:
    nlohmann::json jsonInfo;
    std::vector<std::string> preFiles{};
    std::map<const std::string, size_t> allocateIdxMap;
};
} // namespace Cangjie::CodeCheck

#endif // DATAFLOW_RULE_G_CHK_03_CHECK_H