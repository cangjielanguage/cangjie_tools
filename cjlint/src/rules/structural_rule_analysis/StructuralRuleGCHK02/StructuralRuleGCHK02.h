// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_02_H
#define STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_02_H

#include "nlohmann/json.hpp"
#include "cangjie/AST/Match.h"
#include "cangjie/Basic/Match.h"
#include "rules/structural_rule_analysis/StructuralRule.h"

namespace Cangjie::CodeCheck {
/**
 * G.CHK.02 禁止直接使用外部数据记录日志
 */
class StructuralRuleGCHK02 : public StructuralRule {
public:
    explicit StructuralRuleGCHK02(CodeCheckDiagnosticEngine *diagEngine) : StructuralRule(diagEngine) {};
    ~StructuralRuleGCHK02() override = default;
protected:
    void MatchPattern(ASTContext &ctx, Ptr<Cangjie::AST::Node> node) override;

private:
    struct LogInfo {
        std::string name;
        Position start;
        Position end;
        std::string scopeName;
        bool isPurified;
        friend bool operator < (const LogInfo &a, const LogInfo &b)
        {
            if (a.start.fileID == b.start.fileID) {
                if (a.start == b.start) {
                    return a.name < b.name;
                }
                return a.start < b.start;
            }
            return a.start.fileID < b.start.fileID;
        }
    };

    struct TaintVarInfo {
        std::string name;
        Position start;
        Position end;
        friend bool operator < (const TaintVarInfo &a, const TaintVarInfo &b)
        {
            if (a.start == b.start) {
                return a.name < b.name;
            }
            return a.start < b.start;
        }
    };

    nlohmann::json jsonInfo;
    std::set<LogInfo> externalDataLogSet;
    std::set<TaintVarInfo> TaintVarSet;
    std::vector<std::string> logFunc = { "trace", "debug", "info", "warn", "error", "log" };
    bool isExternalInterfaceFlag = false;
    static bool IsLogObject(const Cangjie::AST::MemberAccess &memberAccess);
    bool IsTaintVar(Ptr<const Cangjie::AST::FuncDecl> funcDecl, const std::string &parentName, const std::string &key);
    void TaintLoggerFinder(Ptr<Cangjie::AST::Node> node);
    void TaintVarFinder(Ptr<Cangjie::AST::Node> node);
    void LoggerFinderHelper(Ptr<const Cangjie::AST::MemberAccess> memberAccess, const Cangjie::AST::CallExpr &callExpr);
    void ObjectFilter(Ptr<Cangjie::AST::Expr> expr, const Position start, const Position end,
        const std::string &scopeName, const std::string refName = "");
    bool ExternalDateCheckerHelper(Cangjie::AST::Decl &decl);
    void AddLogInfoToSet(const Position start, const Position end, const std::string &scopeName,
        const std::string refName);
    void EditLogInfo(const LogInfo &logInfo, bool isPurified);
    void FuncDeclBodyChecker(Ptr<Cangjie::AST::FuncDecl> pFuncDecl, const std::string &refName, const Position start,
        const Position end, const std::string &scopeName);
    void IsExternalInterface(Ptr<Cangjie::AST::Expr> expr);
    void UpdateLogInfoByAssignExpr(const Cangjie::AST::AssignExpr &assignExpr);
    void UpdateLogInfoByMatchExpr(const Cangjie::AST::MatchExpr &matchExpr, Ptr<const Cangjie::AST::CallExpr> callExpr);
    void UpdateLogInfoByMatchExprHelper(const Cangjie::AST::MatchExpr &matchExpr, Ptr<Cangjie::AST::Expr> expr,
        const LogInfo &logInfo);
    static bool IsRegex(Ptr<const Cangjie::AST::CallExpr> callExpr);
    void Purifier(Ptr<Cangjie::AST::Node> node);
};
} // namespace Cangjie::CodeCheck
#endif // STRUCTURAL_RULE_ANALYSIS_STRUCTURAL_RULE_G_CHK_02_H
