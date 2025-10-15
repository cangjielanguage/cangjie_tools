// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DATAFLOW_RULE_P_02_CHECK_H
#define DATAFLOW_RULE_P_02_CHECK_H

#include "../DataflowRule.h"
#include "cangjie/AST/Node.h"
#include "cangjie/Utils/Utils.h"
#include "common/CommonData.h"
#include "common/DiagnosticEngine.h"

namespace Cangjie::CodeCheck {
class DataflowRuleP02Check : public DataflowRule {
public:
    DataflowRuleP02Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleP02Check(CodeCheckDiagnosticEngine *diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleP02Check() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package &package) override;

private:
    void CheckBasedOnCHIRFunc(CHIR::BlockGroup& body);
    struct MemberVarInChain {
        CHIR::Value* base;
        std::string path;
        MemberVarInChain(CHIR::Value* base, std::string path): base(base), path(path) {}
        MemberVarInChain(): base(nullptr), path("") {}
        bool operator==(const MemberVarInChain& rhs) const
        {
            return base == rhs.base && path == rhs.path;
        }
        bool operator<(const MemberVarInChain& rhs) const
        {
            return path < rhs.path;
        }
    };
    using VarWithMutexPosMap = std::map<const CHIR::Value *, std::set<Cangjie::Position>>;
    using VarInChainWithMutexPosMap = std::map<MemberVarInChain, std::set<Cangjie::Position>>;
    using VarWithDiffMutexMap = std::map<const CHIR::Value *, std::map<CHIR::Value *, std::set<Cangjie::Position>>>;
    using VarInChainWithDiffMutexMap = std::map<MemberVarInChain, std::map<CHIR::Value *,
        std::set<Cangjie::Position>>>;
    void CheckNonTerminal(const CHIR::Expression *expr, bool &isLock);
    void CheckSpawnClosure(const CHIR::Lambda *spawnClosure);
    VarWithMutexPosMap varWithoutLock;
    VarInChainWithMutexPosMap varInChainWithoutLock;
    std::set<MemberVarInChain> varInChainStoreInSpawn;
    std::set<const CHIR::Value*> varStoreInSpawn;
    VarWithDiffMutexMap varWithDiffLock;
    VarInChainWithDiffMutexMap varInChainWithDiffLock;
    void AddElementToMap(const CHIR::Value *value, Cangjie::Position pos, bool withLock = false);
    void AddElementToSet(const CHIR::Value *value);
    std::string PrintTips(const VarWithDiffMutexMap &varWithDiffLock);
    bool debug = false;
    CHIR::Value *mutexLock{nullptr};
    template <typename T> MemberVarInChain GetMemberVarInChain(const T* getElementRef);
    MemberVarInChain GetMemberVarInChain(CHIR::Expression *expr);
    template <typename T>
    void CheckApplyOrInVoke(const CHIR::Expression &expr);
    void PrintVarWithoutMutex();
    void PrintVarInChainWithoutMutex();
    void PrintVarWithDiffMutex();
    void PrintVarInChainWithDiffMutex();
    std::pair<Cangjie::Position, Cangjie::Position> FindPositionOfMemberVar(const MemberVarInChain &memberVarInChain);
};
} // namespace Cangjie::CodeCheck
#endif // DATAFLOW_RULE_P_02_CHECK_H
