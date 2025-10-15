// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DATAFLOW_RULE_VAR_01_CHECK_H
#define DATAFLOW_RULE_VAR_01_CHECK_H

#include "../DataflowRule.h"
#include "cangjie/AST/Node.h"
#include "cangjie/Utils/Utils.h"
#include "common/CommonData.h"
#include "common/DiagnosticEngine.h"

#define TRY_GET_EXPR(VAR, EXPR)                                                                                        \
        if (!(VAR)->IsLocalVar()) {                                         \
            return;                                                                                                    \
        }                                                                                                              \
        auto LOCALVAR = StaticCast<CHIR::LocalVar*>(VAR);                                                             \
        auto EXPR = LOCALVAR->GetExpr()

namespace Cangjie::CodeCheck {
class DataflowRuleVAR01Check : public DataflowRule {
public:
    DataflowRuleVAR01Check() : DataflowRule(nullptr) {}
    explicit DataflowRuleVAR01Check(CodeCheckDiagnosticEngine* diagEngine) : DataflowRule(diagEngine) {}
    ~DataflowRuleVAR01Check() override = default;
    void DoAnalysis(CJLintCompilerInstance* instance) override;
protected:
    void CheckBasedOnCHIR(CHIR::Package& package) override;

private:
    using GlobalVarAssignCountMap = std::unordered_map<Ptr<CHIR::GlobalVar>, uint64_t>;
    using LocalVarAssignCountMap = std::unordered_map<Ptr<CHIR::LocalVar>, int>;
    using ClassMemberVarsMap = std::unordered_map<Ptr<CHIR::CustomTypeDef>, std::map<uint64_t, uint64_t>>;
    using PositionPair = std::pair<Cangjie::Position, Cangjie::Position>;

    void CheckBasedOnCHIRFunc(CHIR::BlockGroup& body, ClassMemberVarsMap& memberVarMap);
    void CollectMemberVar(Ptr<CHIR::CustomTypeDef> customTypeDef,
        std::vector<Cangjie::CHIR::MemberVarInfo>& memberVars, ClassMemberVarsMap& memberVarMap);
    void CollectMemberVar(Ptr<CHIR::ClassDef> classDef, ClassMemberVarsMap& memberVarMap);
    void CheckCustomTypeDef(Ptr<CHIR::CustomTypeDef> customTypeDef, ClassMemberVarsMap& memberVarMap);
    void CheckGetElementRef(Ptr<CHIR::Expression> expr, ClassMemberVarsMap& memberVarMap);
    void CheckAllTypesInPackage(CHIR::Package& package, ClassMemberVarsMap& memberVarMap);
    void CheckExprHelper(Ptr<Cangjie::CHIR::Expression> expr, GlobalVarAssignCountMap& globalVarAssignCountMap,
        LocalVarAssignCountMap& localVarAssignCountMap);
    void AddOrEraseElement(Ptr<CHIR::Value> var, GlobalVarAssignCountMap& globalVarAssignCountMap,
        LocalVarAssignCountMap& localVarAssignCountMap);
    void CheckApply(Ptr<CHIR::Expression> expr, LocalVarAssignCountMap& localVarAssignCountMap);
    void CheckApplyHelper(Ptr<CHIR::LocalVar> localVar, LocalVarAssignCountMap& localVarAssignCountMap,
        std::map<std::string, std::set<CHIR::LocalVar*>>& varMap);

    void CheckGlobalVar(const CHIR::GlobalVar* var);
    void CheckLocalVar(const CHIR::LocalVar* var, LocalVarAssignCountMap& localVarAssignCountMap);
    void CheckMemberVarHelper(
        Cangjie::CHIR::CustomType* customTy, std::vector<uint64_t>& path, ClassMemberVarsMap& memberVarMap);
    void CheckMemberVar(const CHIR::Store* store, ClassMemberVarsMap& memberVarMap);
    void CheckMemberVar(const CHIR::StoreElementRef* storeElementRef, ClassMemberVarsMap& memberVarMap);

    std::set<const CHIR::Value*> varHasChanged;
    bool CheckVarHelper(const CHIR::Value* var, int& count);
    std::map<const CHIR::CustomTypeDef*, std::pair<PositionPair, std::string>> genericDefMap;
    std::set<const CHIR::CustomTypeDef*> genericDefSet;
    std::set<const CHIR::GlobalVar*> staticVarInGeneric;
    std::set<const CHIR::GlobalVar*> staticVarhasChanged;
    void PrintDiagnoseInfo(const ClassMemberVarsMap& memberVarMap);
    void PrintDiagnoseInfo(const LocalVarAssignCountMap& localVarAssignCountMap);
    std::set<std::pair<Cangjie::Position, Cangjie::Position>> unsafeBlock;
    void FindAllUnSafeBlock(AST::Package& package);
    bool IsInUnSafeBlock(Cangjie::Position pos);
};
} // namespace Cangjie::CodeCheck
#endif // DATAFLOW_RULE_VAR_01_CHECK_H
