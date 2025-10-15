// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUNC_03_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUNC_03_H
#include <set>
#include <stack>
#include <utility>
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.FUN.03 避免在无关的函数之间重用名字，构成重载
 */
class StructuralRuleGFUNC03 : public StructuralRule {
public:
    explicit StructuralRuleGFUNC03(CodeCheckDiagnosticEngine* diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGFUNC03() override = default;
    enum class Status {
        IN_DIFF_SCOPE,
        SUB_CLASS,
        NONE
    };
    struct FuncInfo {
        std::string identifier;
        std::vector<Ptr<AST::Ty>> paramTys;
        Ptr<AST::Block> block;
        FuncInfo(std::string identifier, std::vector<Ptr<AST::Ty>> paramTys, Ptr<AST::Block> block)
            : identifier(std::move(identifier)), paramTys(std::move(paramTys)), block(block)
        {
        }
        bool operator<(const FuncInfo& other) const
        {
            if (identifier == other.identifier) {
                return paramTys.size() <= other.paramTys.size();
            }
            return identifier < other.identifier;
        }
    };

protected:
    void MatchPattern(ASTContext& ctx, Ptr<Cangjie::AST::Node> node) override;
    Status CheckFuncDecl(std::set<FuncInfo>& allFuncs, FuncInfo target);
    void PrintDiagnoseInfo(FuncInfo& func);

private:
    void CheckFuncOverload(Ptr<Cangjie::AST::Node> node);
    void FuncDeclProcessor(Ptr<Cangjie::AST::Node> node);
    void FileProcessor(Ptr<Cangjie::AST::Node> node);
    std::stack<AST::Block*> blocks;
    std::map<AST::Block*, std::set<FuncInfo>> localFuncInBlock;
    std::set<FuncInfo> allFuncs;
};
} // namespace Cangjie::CodeCheck

#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_FUNC_03_H