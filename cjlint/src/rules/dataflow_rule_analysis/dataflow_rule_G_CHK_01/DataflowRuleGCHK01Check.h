// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DATAFLOW_RULE_G_CHK_01_CHECK_H
#define DATAFLOW_RULE_G_CHK_01_CHECK_H

#include <utility>
#include "common/DiagnosticEngine.h"
#include "common/TaintData.h"
#include "../DataflowRule.h"

namespace Cangjie::CodeCheck {
class DataflowRuleGCHK01Check : public DataflowRule {
public:
    struct NodeInfo {
        bool isRoot = false;
        bool isHarmful = true;
        NodeInfo() {}
        NodeInfo(bool isRoot, bool isHarmful) : isRoot(isRoot), isHarmful(isHarmful) {}
        NodeInfo(const NodeInfo &other)
        {
            isRoot = other.isRoot;
            isHarmful = other.isHarmful;
        }
        NodeInfo &operator = (const NodeInfo &other)
        {
            if (&other != this) {
                this->isRoot = other.isRoot;
                this->isHarmful = other.isHarmful;
            }
            return *this;
        }
    };

    DataflowRuleGCHK01Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleGCHK01Check(CodeCheckDiagnosticEngine *diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleGCHK01Check() override = default;

protected:
    void CheckBasedOnCHIR(CHIR::Package &package) override;

private:
    std::map<std::string, std::set<CHIR::Block *>> varInblockWithoutTainted;
    std::set<std::string> perilousGlobalVars;
    void CheckFuncBody(CHIR::Block& entryBlock);
    template <typename T> bool IsAlarmNotNeeded(T* apply);
    template <typename T> void CheckApply(T* apply, std::set<std::string>& taintedVars);
    void CheckExpr(CHIR::Value* value, CHIR::LocalVar* result, std::set<std::string>& taintedVars);
    template<typename T> void CheckApplyInUsers(T* apply, CHIR::Value* arg);
    template<typename T> void CheckInvokeInUsers(T* invoke, CHIR::Value* arg);
    void CheckStoreInUsers(CHIR::Store* store, std::set<std::string>& taintedVars);
    void CheckApplyUser(CHIR::Expression* user, CHIR::Value* result);
    template <typename T> void CheckApplyUsers(T* apply, std::set<std::string>& taintedVars);
    void GetPerilousGlobalVar(CHIR::GlobalVar *globalVar);
    void CheckStore(CHIR::Store *store, std::set<std::string> &taintedVars);
    void CheckTuple(CHIR::Tuple *tuple, std::set<std::string> &taintedVars);
    void CheckField(CHIR::Field* field, std::set<std::string>& taintedVars);
    void CheckBlock(CHIR::Block* block);
};
} // namespace Cangjie::CodeCheck

#endif // DATAFLOW_RULE_G_CHK_01_CHECK_H
