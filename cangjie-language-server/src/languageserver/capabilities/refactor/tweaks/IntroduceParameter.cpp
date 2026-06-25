// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <map>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <cangjie/AST/Walker.h>
#include "../../../CompilerCangjieProject.h"
#include "../../../common/Inherit/InheritDeclUtil.h"
#include "../../../common/Utils.h"
#include "../TweakRule.h"
#include "../TweakUtils.h"
#include "IntroduceParameter.h"

namespace ark {
const std::unordered_set<Cangjie::AST::ASTKind> CANNOT_INTRODUCE_PARAMETER_EXPR = {
    ASTKind::BLOCK, ASTKind::STR_INTERPOLATION_EXPR, ASTKind::INTERPOLATION_EXPR
};

struct ParamRemovalContext {
    const Tweak::Selection &sel;
    Cangjie::AST::FuncDecl &funcDecl;
    Range &range;
    std::string &paramName;
    std::string &typeName;
    bool &insertedParameter;
    std::vector<std::size_t> &removedParamIndices;
};

struct CallSiteContext {
    const Tweak::Selection &sel;
    Cangjie::AST::FuncDecl &funcDecl;
    Cangjie::AST::FuncDecl &sourceFuncDecl;
    Range &range;
    const std::string &argumentText;
    const std::string &paramName;
    const std::vector<std::size_t> &removedParamIndices;
};

using ArgumentReplacement = std::pair<Range, std::string>;
static Range GetIntroduceParameterExprRange(const SelectionTree &selectionTree, const Range &selectedRange);
static Ptr<Cangjie::AST::Expr> GetIntroduceParameterExpr(
    const SelectionTree &selectionTree, const Range &selectedRange);
static std::string GetIntroduceParameterExprTypeName(
    const SelectionTree &selectionTree, const Range &selectedRange);
// LCOV_EXCL_BR_START
static bool IsLocalVariableTarget(const Ptr<Cangjie::AST::Node> &target)
{
    if (!target || target->astKind != ASTKind::VAR_DECL) {
        return false;
    }
    return !target->TestAnyAttr(
        Attribute::GLOBAL, Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM);
}

static bool IsLocalVariableDeclaredInTargetFunction(
    const Ptr<Cangjie::AST::Node> &target, Cangjie::AST::FuncDecl &funcDecl)
{
    if (!IsLocalVariableTarget(target) || !funcDecl.funcBody || !funcDecl.funcBody->body) {
        return false;
    }
    return funcDecl.funcBody->body->begin <= target->begin && target->end <= funcDecl.funcBody->body->end;
}

static bool CanBuildArgumentAtCallSites(const Tweak::Selection &sel, Cangjie::AST::FuncDecl &funcDecl)
{
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return false;
    }
    auto range = GetIntroduceParameterExprRange(sel.selectionTree, sel.range);
    if (range.start == Position{0, 0, 0} || range.start == range.end) {
        return false;
    }

    bool isValid = true;
    Walker(root->node.get(), [&range, &isValid, &funcDecl](auto node) {
        if (!node || !isValid) {
            return VisitAction::STOP_NOW;
        }
        if (node->begin < range.start || node->end > range.end) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind != ASTKind::REF_EXPR) {
            return VisitAction::WALK_CHILDREN;
        }
        auto refExpr = DynamicCast<Cangjie::AST::RefExpr *>(node.get());
        auto target = refExpr ? refExpr->GetTarget() : nullptr;
        if (IsLocalVariableDeclaredInTargetFunction(target, funcDecl)) {
            isValid = false;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return isValid;
}

static bool HasTargetFunctionLocalVariableReference(const Tweak::Selection &sel, Cangjie::AST::FuncDecl &funcDecl)
{
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return false;
    }
    auto range = GetIntroduceParameterExprRange(sel.selectionTree, sel.range);
    if (range.start == Position{0, 0, 0} || range.start == range.end) {
        return false;
    }

    bool hasLocalRef = false;
    Walker(root->node.get(), [&range, &hasLocalRef, &funcDecl](auto node) {
        if (!node || hasLocalRef) {
            return VisitAction::STOP_NOW;
        }
        if (node->begin < range.start || node->end > range.end) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind != ASTKind::REF_EXPR) {
            return VisitAction::WALK_CHILDREN;
        }
        auto refExpr = DynamicCast<Cangjie::AST::RefExpr *>(node.get());
        auto target = refExpr ? refExpr->GetTarget() : nullptr;
        if (IsLocalVariableDeclaredInTargetFunction(target, funcDecl)) {
            hasLocalRef = true;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return hasLocalRef;
}

static std::vector<TextEdit> RemoveReplacedParameters(const ParamRemovalContext &context);
static void UpdateInheritedFunctionSignatures(
    const Tweak::Selection &sel,
    Cangjie::AST::FuncDecl &funcDecl,
    const std::string &paramName,
    const std::string &typeName,
    const std::vector<std::size_t> &removedParamIndices,
    std::map<std::string, std::vector<TextEdit>> &applyEdits);
static void UpdateCallSites(const CallSiteContext &context, std::map<std::string, std::vector<TextEdit>> &applyEdits);
static void UpdateRelatedFunctionCallSites(
    const Tweak::Selection &sel,
    Cangjie::AST::FuncDecl &funcDecl,
    Range &range,
    const std::string &argumentText,
    const std::string &paramName,
    const std::vector<std::size_t> &removedParamIndices,
    std::map<std::string, std::vector<TextEdit>> &applyEdits);
static std::optional<TextEdit> InsertArgumentAtCallSite(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr);
static std::string BuildCallSiteArgumentText(const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr);
static std::string BuildCompoundAssignArgumentText(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr);
static Ptr<Cangjie::AST::FuncArg> FindCallSiteArgByParam(
    Cangjie::AST::CallExpr &callExpr, Cangjie::AST::FuncParam &param, std::size_t paramIndex);
static std::string GetArgumentTextForParam(
    const CallSiteContext &context,
    Cangjie::AST::CallExpr &callExpr,
    Cangjie::AST::FuncParam &param,
    std::size_t paramIndex);
static std::string GetIdentifierText(const Cangjie::Identifier &identifier);
static std::string GetParamCallName(const Cangjie::AST::FuncParam &param);
static std::string ReplaceParameterNamesWithCallArguments(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr, Cangjie::AST::FuncParamList &paramList);
static std::string ReplaceThisReceiverAtCallSite(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr, const std::string &argumentText);
static std::optional<std::string> GetMemberVariableInitializer(
    const Tweak::Selection &sel, Cangjie::AST::MemberAccess &memberAccess);
static bool ShouldIntroduceNamedParameter(Cangjie::AST::FuncDecl &funcDecl);
static std::string BuildNewParamText(
    Cangjie::AST::FuncDecl &funcDecl, const std::string &paramName, const std::string &typeName);
static std::vector<TextEdit> BuildParameterListSyncEdits(
    Cangjie::AST::FuncDecl &funcDecl,
    const std::string &paramName,
    const std::string &typeName,
    const std::vector<std::size_t> &removedParamIndices);
static std::set<Ptr<Cangjie::AST::Decl>> GetRelatedOverrideDecls(Cangjie::AST::FuncDecl &funcDecl);
static std::set<Ptr<Cangjie::AST::Decl>> CollectLocalOverrideDecls(
    const Tweak::Selection &sel, Cangjie::AST::FuncDecl &funcDecl);

static bool HasExprBoundary(const SelectionTree::SelectionTreeNode &treeNode, const Position &pos, bool isStart)
{
    bool hasBoundary = false;
    SelectionTree::Walk(&treeNode, [&pos, &isStart, &hasBoundary](const SelectionTree::SelectionTreeNode *node) {
        if (!node || !node->node) {
            return SelectionTree::WalkAction::STOP_NOW;
        }
        if (node->node->IsExpr() && (isStart ? node->node->begin == pos : node->node->end == pos)) {
            hasBoundary = true;
            return SelectionTree::WalkAction::STOP_NOW;
        }
        return SelectionTree::WalkAction::WALK_CHILDREN;
    });
    return hasBoundary;
}

static const std::unordered_set<Cangjie::TokenKind> INTRODUCE_PARAMETER_COMPOUND_ASSIGN_OPERATORS = {
    TokenKind::EXP_ASSIGN, TokenKind::MUL_ASSIGN, TokenKind::DIV_ASSIGN, TokenKind::ADD_ASSIGN,
    TokenKind::SUB_ASSIGN, TokenKind::MOD_ASSIGN, TokenKind::LSHIFT_ASSIGN, TokenKind::RSHIFT_ASSIGN,
    TokenKind::AND_ASSIGN, TokenKind::BITXOR_ASSIGN, TokenKind::BITAND_ASSIGN, TokenKind::BITOR_ASSIGN,
    TokenKind::OR_ASSIGN
};

static bool IsSupportedCompoundAssignExpr(const Cangjie::AST::Node &node)
{
    auto assignExpr = DynamicCast<Cangjie::AST::AssignExpr *>(&node);
    return assignExpr && assignExpr->isCompound &&
        INTRODUCE_PARAMETER_COMPOUND_ASSIGN_OPERATORS.count(assignExpr->op) > 0;
}

static std::string GetCompoundAssignFallbackTypeName(const Cangjie::AST::Expr &expr)
{
    auto assignExpr = DynamicCast<const Cangjie::AST::AssignExpr *>(&expr);
    if (!assignExpr || !assignExpr->leftValue || !assignExpr->leftValue->GetTy() ||
        GetString(*assignExpr->leftValue->GetTy()) == "UnknownType") {
        return "";
    }
    return GetString(*assignExpr->leftValue->GetTy());
}

static bool HasValidIntroduceParameterExprType(const Cangjie::AST::Expr &expr)
{
    if (expr.GetTy() && GetString(*expr.GetTy()) != "UnknownType") {
        return true;
    }
    return IsSupportedCompoundAssignExpr(expr) && !GetCompoundAssignFallbackTypeName(expr).empty();
}

static Ptr<Cangjie::AST::Expr> GetContainingIntroduceParameterExpr(
    const SelectionTree &selectionTree, const Range &selectedRange)
{
    Ptr<Cangjie::AST::Expr> containingExpr = nullptr;
    auto root = selectionTree.root();
    if (!root || !root->node) {
        return nullptr;
    }
    SelectionTree::Walk(root, [&selectedRange, &containingExpr](const SelectionTree::SelectionTreeNode *treeNode) {
        if (!treeNode || !treeNode->node) {
            return SelectionTree::WalkAction::STOP_NOW;
        }
        if (!treeNode->node->IsExpr() || treeNode->node->begin > selectedRange.start ||
            treeNode->node->end < selectedRange.end) {
            return SelectionTree::WalkAction::WALK_CHILDREN;
        }
        bool isSupportedExpr = treeNode->node->astKind == ASTKind::BINARY_EXPR ||
            IsSupportedCompoundAssignExpr(*treeNode->node);
        if (isSupportedExpr && treeNode->node->end == selectedRange.end &&
            HasExprBoundary(*treeNode, selectedRange.start, true) &&
            HasExprBoundary(*treeNode, selectedRange.end, false)) {
            containingExpr = DynamicCast<Cangjie::AST::Expr *>(treeNode->node.get());
        }
        return SelectionTree::WalkAction::WALK_CHILDREN;
    });
    return containingExpr;
}

static Range GetIntroduceParameterExprRange(const SelectionTree &selectionTree, const Range &selectedRange)
{
    Range range;
    auto exprNode = TweakUtils::GetCompleteExprNode(selectionTree, selectedRange);
    if (!exprNode || !exprNode->node) {
        auto expr = GetContainingIntroduceParameterExpr(selectionTree, selectedRange);
        if (!expr || !HasValidIntroduceParameterExprType(*expr)) {
            return range;
        }
        return selectedRange;
    }
    range.start = exprNode->node->begin;
    range.end = exprNode->node->end;
    return range;
}

static Ptr<Cangjie::AST::Expr> GetIntroduceParameterExpr(const SelectionTree &selectionTree,
    const Range &selectedRange)
{
    auto exprNode = TweakUtils::GetCompleteExprNode(selectionTree, selectedRange);
    if (exprNode && exprNode->node) {
        return DynamicCast<Cangjie::AST::Expr *>(exprNode->node.get());
    }
    return GetContainingIntroduceParameterExpr(selectionTree, selectedRange);
}

static std::string GetIntroduceParameterExprTypeName(const SelectionTree &selectionTree,
    const Range &selectedRange)
{
    auto expr = GetIntroduceParameterExpr(selectionTree, selectedRange);
    if (expr && IsSupportedCompoundAssignExpr(*expr)) {
        return GetCompoundAssignFallbackTypeName(*expr);
    }
    auto exactTypeName = TweakUtils::GetSelectedExprTypeName(selectionTree, selectedRange);
    if (!exactTypeName.empty()) {
        return exactTypeName;
    }
    if (!expr || !expr->GetTy() || GetString(*expr->GetTy()) == "UnknownType") {
        return "";
    }
    return GetString(*expr->GetTy());
}

static std::string GetExplicitTypeName(const Tweak::Selection &sel)
{
    auto it = sel.extraOptions.find("typeName");
    return it == sel.extraOptions.end() ? "" : it->second;
}

bool IntroducedParameterTypeBreaksPublicSignature(Cangjie::AST::FuncDecl &funcDecl, Ptr<Cangjie::AST::Ty> ty)
{
    if (!funcDecl.TestAttr(Attribute::PUBLIC) || !ty) {
        return false;
    }
    auto decl = Ty::GetDeclPtrOfTy(ty);
    return decl && !decl->IsExportedDecl();
}

namespace {
// NOLINTNEXTLINE(G.NAM.02-CPP)
class IntroduceParameterRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (!root || !root->node) {
            return false;
        }
        auto range = GetIntroduceParameterExprRange(sel.selectionTree, sel.range);
        auto selectedExpr = GetIntroduceParameterExpr(sel.selectionTree, sel.range);
        if (range.start == Position{0, 0, 0} || range.start == range.end || !selectedExpr) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceParameter::IntroduceParameterError::FAIL_GET_ROOT_EXPR))));
            return false;
        }
        if (CANNOT_INTRODUCE_PARAMETER_EXPR.count(selectedExpr->astKind)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(
                    static_cast<int>(IntroduceParameter::IntroduceParameterError::INVALID_CODE_SEGMENT))));
            return false;
        }
        auto funcDecl = IntroduceParameter::GetTargetFunc(sel);
        if (!funcDecl) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceParameter::IntroduceParameterError::INVALID_SCOPE))));
            return false;
        }
        if (IntroduceParameter::IsMemberAssignInInit(funcDecl, selectedExpr)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(
                    static_cast<int>(IntroduceParameter::IntroduceParameterError::MEMBER_ASSIGN_IN_CONSTRUCTOR))));
            return false;
        }
        if (GetIntroduceParameterExprTypeName(sel.selectionTree, range).empty()) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceParameter::IntroduceParameterError::INVALID_TYPE))));
            return false;
        }
        if (selectedExpr && IntroducedParameterTypeBreaksPublicSignature(*funcDecl, selectedExpr->GetTy())) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(
                    IntroduceParameter::IntroduceParameterError::PUBLIC_DECL_USES_NON_PUBLIC_TYPE))));
            return false;
        }
        return true;
    }
};
}

bool IntroduceParameter::Prepare(const Selection &sel)
{
    TweakRuleEngine ruleEngine;
    ruleEngine.AddRule(std::make_unique<IntroduceParameterRule>());
    ruleEngine.CheckRules(sel, extraOptions);
    return true;
}

std::optional<Tweak::Effect> IntroduceParameter::Apply(const Selection &sel)
{
    Effect effect;
    if (!sel.arkAst || !sel.arkAst->file || !sel.arkAst->sourceManager) {
        return std::nullopt;
    }
    auto funcDecl = GetTargetFunc(sel);
    if (!funcDecl) {
        return std::nullopt;
    }

    std::string paramName = "newParameter";
    auto nameIt = sel.extraOptions.find("suggestName");
    if (nameIt != sel.extraOptions.end()) {
        paramName = nameIt->second;
    }

    Range range = GetIntroduceParameterExprRange(sel.selectionTree, sel.range);
    if (range.start == Position{0, 0, 0} || range.start == range.end) {
        return std::nullopt;
    }

    std::string typeName = GetIntroduceParameterExprTypeName(sel.selectionTree, sel.range);
    if (typeName.empty()) {
        typeName = GetExplicitTypeName(sel);
    }
    if (typeName.empty()) {
        return std::nullopt;
    }
    std::vector<TextEdit> textEdits;
    bool insertedParameter = false;
    std::vector<std::size_t> removedParamIndices;
    ParamRemovalContext removalContext{sel, *funcDecl, range, paramName, typeName, insertedParameter,
        removedParamIndices};
    auto removeParamEdits = RemoveReplacedParameters(removalContext);
    textEdits.insert(textEdits.end(), removeParamEdits.begin(), removeParamEdits.end());
    if (!insertedParameter) {
        textEdits.push_back(InsertParameter(*funcDecl, paramName, typeName));
    }
    textEdits.push_back(ReplaceExprWithParam(sel, range, paramName));

    std::string uri = URI::URIFromAbsolutePath(sel.arkAst->file->filePath).ToString();
    effect.applyEdits.emplace(uri, std::move(textEdits));
    UpdateInheritedFunctionSignatures(sel, *funcDecl, paramName, typeName, removedParamIndices, effect.applyEdits);
    std::string argumentText = sel.arkAst->sourceManager->GetContentBetween(range.start, range.end);
    if (!HasTargetFunctionLocalVariableReference(sel, *funcDecl) && CanBuildArgumentAtCallSites(sel, *funcDecl)) {
        UpdateRelatedFunctionCallSites(
            sel, *funcDecl, range, argumentText, paramName, removedParamIndices, effect.applyEdits);
    }
    return std::move(effect);
}

std::map<std::string, std::string> IntroduceParameter::ExtraOptions()
{
    return extraOptions;
}

Ptr<Cangjie::AST::FuncDecl> IntroduceParameter::GetTargetFunc(const Selection &sel)
{
    return TweakUtils::GetTargetFunc(
        sel.selectionTree, sel.arkAst, GetIntroduceParameterExprRange(sel.selectionTree, sel.range));
}

bool IntroduceParameter::IsMemberAssignInInit(Ptr<FuncDecl> func, Ptr<Node> expr)
{
    if (!expr || !func || !func->TestAttr(Attribute::CONSTRUCTOR)) {
        return false;
    }

    if (auto assign = DynamicCast<AssignExpr>(expr)) {
        if (auto leftValue = DynamicCast<MemberAccess>(assign->leftValue.get())) {
            if (auto refExpr = DynamicCast<RefExpr>(leftValue->baseExpr.get())) {
                return refExpr->isThis;
            }
        }
    }

    return false;
}

TextEdit IntroduceParameter::InsertParameter(
    Cangjie::AST::FuncDecl &funcDecl, std::string &paramName, std::string &typeName)
{
    TextEdit textEdit;
    if (!funcDecl.funcBody || funcDecl.funcBody->paramLists.empty() || !funcDecl.funcBody->paramLists.front()) {
        return textEdit;
    }

    auto paramList = funcDecl.funcBody->paramLists.front().get();
    std::string paramText = BuildNewParamText(funcDecl, paramName, typeName);
    textEdit.range = TransformFromChar2IDE({paramList->rightParenPos, paramList->rightParenPos});
    textEdit.newText = paramList->params.empty() ? paramText : ", " + paramText;
    return textEdit;
}

static bool ShouldIntroduceNamedParameter(Cangjie::AST::FuncDecl &funcDecl)
{
    if (!funcDecl.funcBody || funcDecl.funcBody->paramLists.empty() || !funcDecl.funcBody->paramLists.front()) {
        return false;
    }
    const auto &params = funcDecl.funcBody->paramLists.front()->params;
    return std::any_of(params.begin(), params.end(), [](const auto &param) {
        return param && param->isNamedParam;
    });
}

static std::string BuildNewParamText(
    Cangjie::AST::FuncDecl &funcDecl, const std::string &paramName, const std::string &typeName)
{
    return paramName + (ShouldIntroduceNamedParameter(funcDecl) ? "!: " : ": ") + typeName;
}

static bool IsFuncParamUsedOutsideRange(Cangjie::AST::FuncDecl &funcDecl, Cangjie::AST::FuncParam &param, Range &range)
{
    if (!funcDecl.funcBody || !funcDecl.funcBody->body) {
        return true;
    }

    bool isUsed = false;
    Walker(funcDecl.funcBody->body.get(), [&param, &range, &isUsed](auto node) {
        if (!node || isUsed) {
            return VisitAction::STOP_NOW;
        }
        if (!(node->begin < range.start) && !(range.end < node->end)) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind != ASTKind::REF_EXPR) {
            return VisitAction::WALK_CHILDREN;
        }
        auto refExpr = DynamicCast<Cangjie::AST::RefExpr *>(node.get());
        if (refExpr && refExpr->GetTarget().get() == &param) {
            isUsed = true;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return isUsed;
}
// LCOV_EXCL_BR_STOP
// LCOV_EXCL_START
static bool GetParamRemovalRange(Cangjie::AST::FuncParamList &paramList, Cangjie::AST::FuncParam &param, Range &range)
{
    for (std::size_t index = 0; index < paramList.params.size(); ++index) {
        if (!paramList.params[index] || paramList.params[index].get() != &param) {
            continue;
        }
        if (paramList.params.size() == 1) {
            range.start = param.begin;
            range.end = param.end;
            return true;
        }
        if (index + 1 < paramList.params.size() && paramList.params[index + 1]) {
            range.start = param.begin;
            range.end = paramList.params[index + 1]->begin;
            return true;
        }
        if (index > 0 && paramList.params[index - 1]) {
            range.start = paramList.params[index - 1]->end;
            range.end = param.end;
            return true;
        }
        return false;
    }
    return false;
}

static std::vector<std::size_t> CollectRemovableParamIndices(
    const Tweak::Selection &sel, Cangjie::AST::FuncDecl &funcDecl, Cangjie::AST::FuncParamList &paramList, Range &range)
{
    std::vector<std::size_t> indices;
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return indices;
    }

    Walker(root->node.get(), [&range, &funcDecl, &paramList, &indices](auto node) {
        if (!node) {
            return VisitAction::STOP_NOW;
        }
        if (node->end < range.start || node->begin > range.end) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind != ASTKind::REF_EXPR) {
            return VisitAction::WALK_CHILDREN;
        }
        if (node->begin < range.start || node->end > range.end) {
            return VisitAction::WALK_CHILDREN;
        }
        auto refExpr = DynamicCast<Cangjie::AST::RefExpr *>(node.get());
        auto target = refExpr ? DynamicCast<Cangjie::AST::FuncParam *>(refExpr->GetTarget().get()) : nullptr;
        if (!target || IsFuncParamUsedOutsideRange(funcDecl, *target, range)) {
            return VisitAction::WALK_CHILDREN;
        }
        for (std::size_t index = 0; index < paramList.params.size(); ++index) {
            if (!paramList.params[index] || paramList.params[index].get() != target ||
                std::find(indices.begin(), indices.end(), index) != indices.end()) {
                continue;
            }
            indices.push_back(index);
            break;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();

    std::sort(indices.begin(), indices.end());
    return indices;
}

static TextEdit BuildReplacementParameterEdit(
    Cangjie::AST::FuncParamList &paramList, std::size_t index, const std::string &newParamText)
{
    TextEdit replaceEdit;
    replaceEdit.range = TransformFromChar2IDE({paramList.params[index]->begin, paramList.params[index]->end});
    replaceEdit.newText = newParamText;
    return replaceEdit;
}

static std::vector<TextEdit> BuildPartialParameterRemovalEdits(
    Cangjie::AST::FuncParamList &paramList, const std::vector<std::size_t> &removedParamIndices,
    std::size_t replacedIndex)
{
    std::vector<TextEdit> edits;
    for (auto it = removedParamIndices.rbegin(); it != removedParamIndices.rend(); ++it) {
        if (*it == replacedIndex || *it >= paramList.params.size() || !paramList.params[*it]) {
            continue;
        }
        Range removeRange;
        if (!GetParamRemovalRange(paramList, *paramList.params[*it], removeRange)) {
            continue;
        }
        TextEdit textEdit;
        textEdit.range = TransformFromChar2IDE(removeRange);
        edits.push_back(textEdit);
    }
    return edits;
}
// LCOV_EXCL_STOP
static std::vector<TextEdit> BuildAllParameterReplacementEdit(
    Cangjie::AST::FuncParamList &paramList, const std::string &newParamText)
{
    TextEdit textEdit;
    Range replaceRange = {paramList.params.front()->begin, paramList.params.back()->end};
    textEdit.range = TransformFromChar2IDE(replaceRange);
    textEdit.newText = newParamText;
    return {textEdit};
}

static std::vector<TextEdit> RemoveReplacedParameters(const ParamRemovalContext &context)
{
    std::vector<TextEdit> edits;
    if (!context.funcDecl.funcBody || context.funcDecl.funcBody->paramLists.empty() ||
        !context.funcDecl.funcBody->paramLists.front()) {
        return edits;
    }

    auto paramList = context.funcDecl.funcBody->paramLists.front().get();
    context.removedParamIndices =
        CollectRemovableParamIndices(context.sel, context.funcDecl, *paramList, context.range);
    if (context.removedParamIndices.empty()) {
        return edits;
    }

    std::string newParamText = BuildNewParamText(context.funcDecl, context.paramName, context.typeName);
    if (context.removedParamIndices.size() == paramList->params.size()) {
        context.insertedParameter = true;
        return BuildAllParameterReplacementEdit(*paramList, newParamText);
    }

    std::size_t replacedIndex = context.removedParamIndices.front();
    if (replacedIndex >= paramList->params.size() || !paramList->params[replacedIndex]) {
        return edits;
    }
    edits.push_back(BuildReplacementParameterEdit(*paramList, replacedIndex, newParamText));
    context.insertedParameter = true;

    auto removeEdits = BuildPartialParameterRemovalEdits(*paramList, context.removedParamIndices, replacedIndex);
    edits.insert(edits.end(), removeEdits.begin(), removeEdits.end());
    return edits;
}

static std::vector<TextEdit> BuildParameterListSyncEdits(
    Cangjie::AST::FuncDecl &funcDecl,
    const std::string &paramName,
    const std::string &typeName,
    const std::vector<std::size_t> &removedParamIndices)
{
    std::vector<TextEdit> edits;
    if (!funcDecl.funcBody || funcDecl.funcBody->paramLists.empty() || !funcDecl.funcBody->paramLists.front()) {
        return edits;
    }

    auto paramList = funcDecl.funcBody->paramLists.front().get();
    if (removedParamIndices.empty()) {
        auto name = paramName;
        auto type = typeName;
        edits.push_back(IntroduceParameter::InsertParameter(funcDecl, name, type));
        return edits;
    }

    std::string newParamText = BuildNewParamText(funcDecl, paramName, typeName);
    if (removedParamIndices.size() == paramList->params.size()) {
        return BuildAllParameterReplacementEdit(*paramList, newParamText);
    }

    std::size_t replacedIndex = removedParamIndices.front();
    if (replacedIndex >= paramList->params.size() || !paramList->params[replacedIndex]) {
        return edits;
    }
    edits.push_back(BuildReplacementParameterEdit(*paramList, replacedIndex, newParamText));
    auto removeEdits = BuildPartialParameterRemovalEdits(*paramList, removedParamIndices, replacedIndex);
    edits.insert(edits.end(), removeEdits.begin(), removeEdits.end());
    return edits;
}

static void UpdateInheritedFunctionSignatures(
    const Tweak::Selection &sel,
    Cangjie::AST::FuncDecl &funcDecl,
    const std::string &paramName,
    const std::string &typeName,
    const std::vector<std::size_t> &removedParamIndices,
    std::map<std::string, std::vector<TextEdit>> &applyEdits)
{
    auto inheritedDecls = GetRelatedOverrideDecls(funcDecl);
    auto localOverrides = CollectLocalOverrideDecls(sel, funcDecl);
    inheritedDecls.insert(localOverrides.begin(), localOverrides.end());
    for (auto &decl : inheritedDecls) {
        auto relatedFunc = DynamicCast<Cangjie::AST::FuncDecl *>(decl.get());
        if (!relatedFunc || relatedFunc == &funcDecl || !relatedFunc->curFile) {
            continue;
        }
        auto edits = BuildParameterListSyncEdits(*relatedFunc, paramName, typeName, removedParamIndices);
        if (edits.empty()) {
            continue;
        }
        std::string uri = URI::URIFromAbsolutePath(relatedFunc->curFile->filePath).ToString();
        auto &fileEdits = applyEdits[uri];
        fileEdits.insert(fileEdits.end(), edits.begin(), edits.end());
    }
}

static std::set<Ptr<Cangjie::AST::Decl>> GetRelatedOverrideDecls(Cangjie::AST::FuncDecl &funcDecl)
{
    InheritDeclUtil inherit(&funcDecl);
    inherit.HandleFuncDecl(false);
    return inherit.GetRelatedFuncDecls();
}

static std::set<Ptr<Cangjie::AST::Decl>> CollectLocalOverrideDecls(
    const Tweak::Selection &sel, Cangjie::AST::FuncDecl &funcDecl)
{
    std::set<Ptr<Cangjie::AST::Decl>> decls;
    if (!sel.arkAst || !sel.arkAst->file || !sel.arkAst->file->curPackage) {
        return decls;
    }
    for (auto &file : sel.arkAst->file->curPackage->files) {
        if (!file) {
            continue;
        }
        Walker(file.get(), [&funcDecl, &decls](auto node) {
            auto relatedFunc = DynamicCast<Cangjie::AST::FuncDecl *>(node.get());
            if (!relatedFunc || relatedFunc == &funcDecl || relatedFunc->identifier != funcDecl.identifier ||
                !relatedFunc->TestAttr(Attribute::OVERRIDE)) {
                return VisitAction::WALK_CHILDREN;
            }
            decls.insert(relatedFunc);
            return VisitAction::WALK_CHILDREN;
        }).Walk();
    }
    return decls;
}

TextEdit IntroduceParameter::ReplaceExprWithParam(const Selection &sel, Range &range, std::string &paramName)
{
    TextEdit textEdit;
    auto selectedExpr = GetIntroduceParameterExpr(sel.selectionTree, range);
    bool isCompoundAssign = selectedExpr && IsSupportedCompoundAssignExpr(*selectedExpr);
    textEdit.newText = isCompoundAssign ? "" : paramName;
    if (isCompoundAssign) {
        Range deleteLineRange = range;
        deleteLineRange.start.column = 1;
        deleteLineRange.end.line += 1;
        deleteLineRange.end.column = 1;
        textEdit.range = TransformFromChar2IDE(deleteLineRange);
        return textEdit;
    }

    std::string charContent = sel.arkAst->sourceManager->GetContentBetween(
        {range.start.fileID, range.start.line, 1}, range.start);
    range.start.column = CountUnicodeCharacters(charContent) + 1;
    
    std::string endCharContent = sel.arkAst->sourceManager->GetContentBetween(
        {range.end.fileID, range.end.line, 1}, range.end);
    range.end.column = CountUnicodeCharacters(endCharContent) + 1;
    
    textEdit.range = TransformFromChar2IDE(range);
    return textEdit;
}

static void UpdateCallSites(const CallSiteContext &context, std::map<std::string, std::vector<TextEdit>> &applyEdits)
{
    if (!context.sel.arkAst || !context.sel.arkAst->file || !context.sel.arkAst->file->curPackage) {
        return;
    }

    for (auto &file : context.sel.arkAst->file->curPackage->files) {
        if (!file) {
            continue;
        }
        std::vector<TextEdit> callSiteEdits;
        Walker(file.get(), [&context, &callSiteEdits](auto node) {
            if (!node || node->astKind != ASTKind::CALL_EXPR) {
                return VisitAction::WALK_CHILDREN;
            }
            auto callExpr = DynamicCast<Cangjie::AST::CallExpr *>(node.get());
            if (!callExpr || callExpr->resolvedFunction.get() != &context.funcDecl) {
                return VisitAction::WALK_CHILDREN;
            }
            auto textEdit = InsertArgumentAtCallSite(context, *callExpr);
            if (textEdit) {
                callSiteEdits.push_back(*textEdit);
            }
            return VisitAction::WALK_CHILDREN;
        }).Walk();

        if (callSiteEdits.empty()) {
            continue;
        }
        std::string uri = URI::URIFromAbsolutePath(file->filePath).ToString();
        auto &edits = applyEdits[uri];
        edits.insert(edits.end(), callSiteEdits.begin(), callSiteEdits.end());
    }
}

static void UpdateRelatedFunctionCallSites(
    const Tweak::Selection &sel,
    Cangjie::AST::FuncDecl &funcDecl,
    Range &range,
    const std::string &argumentText,
    const std::string &paramName,
    const std::vector<std::size_t> &removedParamIndices,
    std::map<std::string, std::vector<TextEdit>> &applyEdits)
{
    CallSiteContext primaryContext{sel, funcDecl, funcDecl, range, argumentText, paramName, removedParamIndices};
    UpdateCallSites(primaryContext, applyEdits);

    auto inheritedDecls = GetRelatedOverrideDecls(funcDecl);
    auto localOverrides = CollectLocalOverrideDecls(sel, funcDecl);
    inheritedDecls.insert(localOverrides.begin(), localOverrides.end());
    for (auto &decl : inheritedDecls) {
        auto relatedFunc = DynamicCast<Cangjie::AST::FuncDecl *>(decl.get());
        if (!relatedFunc || relatedFunc == &funcDecl) {
            continue;
        }
        CallSiteContext relatedContext{sel, *relatedFunc, funcDecl, range, argumentText, paramName, removedParamIndices};
        UpdateCallSites(relatedContext, applyEdits);
    }
}

static bool HasNamedArg(Cangjie::AST::CallExpr &callExpr)
{
    for (const auto &arg : callExpr.args) {
        if (arg && !arg->name.Empty()) {
            return true;
        }
    }
    return false;
}

static ArkAST *GetCallSiteAst(const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr)
{
    if (!callExpr.curFile) {
        return context.sel.arkAst;
    }
    if (context.sel.arkAst && context.sel.arkAst->file &&
        context.sel.arkAst->file.get() == callExpr.curFile.get()) {
        return context.sel.arkAst;
    }
    return CompilerCangjieProject::GetInstance()->GetArkAST(callExpr.curFile->filePath);
}

static Range TransformCallSiteRangeToIDE(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr, Range range)
{
    auto ast = GetCallSiteAst(context, callExpr);
    if (ast && ast->file) {
        PositionUTF8ToIDE(ast->tokens, range.start, callExpr);
        PositionUTF8ToIDE(ast->tokens, range.end, callExpr);
    }
    return TransformFromChar2IDE(range);
}

static bool IsRemovedCallArg(Ptr<Cangjie::AST::FuncArg> arg, const std::vector<Ptr<Cangjie::AST::FuncArg>> &removedArgs)
{
    return std::find(removedArgs.begin(), removedArgs.end(), arg) != removedArgs.end();
}

static std::string GetCallArgText(const CallSiteContext &context, Cangjie::AST::FuncArg &arg)
{
    if (!context.sel.arkAst || !context.sel.arkAst->sourceManager) {
        return "";
    }
    return context.sel.arkAst->sourceManager->GetContentBetween(arg.begin, arg.end);
}

static std::optional<TextEdit> ReplaceRemovedCallArguments(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr, const std::string &newArgument, bool hasNamedArg)
{
// LCOV_EXCL_BR_START
    if (context.removedParamIndices.empty() || !context.funcDecl.funcBody ||
        context.funcDecl.funcBody->paramLists.empty() || !context.funcDecl.funcBody->paramLists.front()) {
        return std::nullopt;
    }

    auto paramList = context.funcDecl.funcBody->paramLists.front().get();
    std::vector<Ptr<Cangjie::AST::FuncArg>> removedArgs;
    for (auto index : context.removedParamIndices) {
        if (index >= paramList->params.size() || !paramList->params[index]) {
            return std::nullopt;
        }
        auto arg = FindCallSiteArgByParam(callExpr, *paramList->params[index], index);
        if (!arg) {
            return std::nullopt;
        }
        removedArgs.push_back(arg);
    }
    std::sort(removedArgs.begin(), removedArgs.end(), [](const auto &left, const auto &right) {
        return left && right && left->begin < right->begin;
    });

    TextEdit textEdit;
    textEdit.range = TransformCallSiteRangeToIDE(
        context, callExpr, {removedArgs.front()->begin, removedArgs.back()->end});
    std::ostringstream replacement;
    bool insertedNewArgument = false;
    bool needSeparator = false;
    for (const auto &arg : callExpr.args) {
        if (!arg || arg->begin < removedArgs.front()->begin || arg->end > removedArgs.back()->end) {
            continue;
        }
        if (IsRemovedCallArg(arg.get(), removedArgs)) {
            if (insertedNewArgument) {
                continue;
            }
            if (needSeparator) {
                replacement << ", ";
            }
            replacement << (hasNamedArg ? context.paramName + ": " : "") << newArgument;
            insertedNewArgument = true;
            needSeparator = true;
            continue;
        }
        if (needSeparator) {
            replacement << ", ";
        }
        replacement << GetCallArgText(context, *arg);
        needSeparator = true;
    }
    textEdit.newText = replacement.str();
    return textEdit;
// LCOV_EXCL_BR_STOP
}

static std::optional<Position> FindMatchingRightParenFrom(
    const Tweak::Selection &sel, const Position &leftParenPos, const Position &searchEnd)
{
    if (!sel.arkAst || !sel.arkAst->sourceManager || leftParenPos.IsZero() || searchEnd.IsZero() ||
        searchEnd <= leftParenPos) {
        return std::nullopt;
    }

    std::string suffix = sel.arkAst->sourceManager->GetContentBetween(leftParenPos, searchEnd);
    int depth = 0;
    for (size_t offset = 0; offset < suffix.size(); ++offset) {
        if (suffix[offset] == '(') {
            ++depth;
            continue;
        }
        if (suffix[offset] != ')') {
            continue;
        }
        --depth;
        if (depth == 0) {
            return TweakUtils::PositionAtOffset(leftParenPos, suffix, offset);
        }
    }
    return std::nullopt;
}

static std::string GetCallName(Cangjie::AST::CallExpr &callExpr)
{
    if (!callExpr.baseFunc) {
        return "";
    }
    if (auto refExpr = DynamicCast<Cangjie::AST::RefExpr *>(callExpr.baseFunc.get())) {
        return GetIdentifierText(refExpr->ref.identifier);
    }
    if (auto memberAccess = DynamicCast<Cangjie::AST::MemberAccess *>(callExpr.baseFunc.get())) {
        return memberAccess->field.GetRawText();
    }
    return "";
}

static std::optional<Position> FindCallRightParenByName(
    const Tweak::Selection &sel, Cangjie::AST::CallExpr &callExpr)
{
    if (!sel.arkAst || !sel.arkAst->sourceManager || !callExpr.baseFunc || callExpr.baseFunc->end.IsZero()) {
        return std::nullopt;
    }
    std::string callName = GetCallName(callExpr);
    if (callName.empty()) {
        return std::nullopt;
    }
    Position lineStart{callExpr.baseFunc->end.fileID, callExpr.baseFunc->end.line, 1};
    Position lineEnd{callExpr.baseFunc->end.fileID, callExpr.baseFunc->end.line + 1, 1};
    std::string lineText = sel.arkAst->sourceManager->GetContentBetween(lineStart, lineEnd);
    std::string needle = callName + "(";

    std::optional<Position> result;
    size_t cursor = 0;
    while (cursor < lineText.size()) {
        size_t pos = lineText.find(needle, cursor);
        if (pos == std::string::npos) {
            break;
        }
        Position namePos = TweakUtils::PositionAtOffset(lineStart, lineText, pos);
        if (namePos < callExpr.baseFunc->begin || namePos > callExpr.baseFunc->end) {
            cursor = pos + needle.size();
            continue;
        }
        Position leftParenPos = TweakUtils::PositionAtOffset(lineStart, lineText, pos + callName.size());
        if (auto rightParen = FindMatchingRightParenFrom(sel, leftParenPos, lineEnd)) {
            result = rightParen;
        }
        cursor = pos + needle.size();
    }
    return result;
}

static std::optional<Position> ResolveCallArgumentInsertPosition(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr)
{
    if (!callExpr.leftParenPos.IsZero()) {
        Position searchEnd = callExpr.end.IsZero() ? callExpr.rightParenPos : callExpr.end;
        if (auto rightParen = FindMatchingRightParenFrom(context.sel, callExpr.leftParenPos, searchEnd)) {
            return rightParen;
        }
    }
    if (auto rightParen = FindCallRightParenByName(context.sel, callExpr)) {
        return rightParen;
    }
    return std::nullopt;
}

// LCOV_EXCL_START
static TextEdit InsertNewCallArgument(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr, const std::string &newArgument, bool hasNamedArg)
{
    TextEdit textEdit;
    bool introduceNamed = ShouldIntroduceNamedParameter(context.funcDecl);
    std::ostringstream insertText;
    Position insertPos = ResolveCallArgumentInsertPosition(context, callExpr).value_or(callExpr.rightParenPos);
    textEdit.range = TransformCallSiteRangeToIDE(
        context, callExpr, {insertPos, insertPos});
    if (!callExpr.args.empty()) {
        insertText << ", ";
    }
    if (introduceNamed || hasNamedArg) {
        insertText << context.paramName << ": ";
    }
    insertText << newArgument;
    textEdit.newText = insertText.str();
    return textEdit;
}
// LCOV_EXCL_STOP
static std::optional<TextEdit> InsertArgumentAtCallSite(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr)
{
    if (callExpr.rightParenPos.IsZero()) {
        return std::nullopt;
    }

    bool hasNamedArg = HasNamedArg(callExpr);
    std::string newArgument = BuildCallSiteArgumentText(context, callExpr);
    if (auto textEdit = ReplaceRemovedCallArguments(context, callExpr, newArgument, hasNamedArg)) {
        return textEdit;
    }
    return InsertNewCallArgument(context, callExpr, newArgument, hasNamedArg);
}
// LCOV_EXCL_BR_START
static std::optional<ArgumentReplacement> BuildReferenceReplacement(
    const CallSiteContext &context,
    Cangjie::AST::CallExpr &callExpr,
    Cangjie::AST::FuncParamList &paramList,
    Cangjie::AST::RefExpr &refExpr)
{
    auto target = DynamicCast<Cangjie::AST::FuncParam *>(refExpr.GetTarget().get());
    if (!target) {
        return std::nullopt;
    }

    for (std::size_t index = 0; index < paramList.params.size(); ++index) {
        if (!paramList.params[index] || paramList.params[index].get() != target) {
            continue;
        }
        std::string argText = GetArgumentTextForParam(context, callExpr, *paramList.params[index], index);
        if (argText.empty()) {
            return std::nullopt;
        }
        Range refRange = {refExpr.begin, refExpr.end};
        if (refRange.start < context.range.start || refRange.end > context.range.end) {
            return std::nullopt;
        }
        return ArgumentReplacement{refRange, argText};
    }
    return std::nullopt;
}

static std::optional<ArgumentReplacement> TryGetMemberAccessReplacement(
    const CallSiteContext &context, Ptr<Cangjie::AST::Node> &node)
{
    if (node->astKind != ASTKind::MEMBER_ACCESS) {
        return std::nullopt;
    }

    auto memberAccess = DynamicCast<Cangjie::AST::MemberAccess *>(node.get());
    if (!memberAccess) {
        return std::nullopt;
    }

    if (node->begin < context.range.start || node->end > context.range.end) {
        return std::nullopt;
    }

    auto initValue = GetMemberVariableInitializer(context.sel, *memberAccess);
    if (!initValue) {
        return std::nullopt;
    }

    Range memberRange = {memberAccess->begin, memberAccess->end};
    return ArgumentReplacement{memberRange, *initValue};
}

static std::vector<ArgumentReplacement> CollectCallSiteArgumentReplacements(
    const CallSiteContext &context,
    Cangjie::AST::CallExpr &callExpr,
    Cangjie::AST::FuncParamList &paramList)
{
    std::vector<ArgumentReplacement> replacements;
    auto root = context.sel.selectionTree.root();
    if (!root || !root->node) {
        return replacements;
    }

    Walker(root->node.get(), [&context, &paramList, &callExpr, &replacements](auto node) {
        if (!node) {
            return VisitAction::STOP_NOW;
        }
        if (node->end < context.range.start || node->begin > context.range.end) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (auto replacement = TryGetMemberAccessReplacement(context, node)) {
            replacements.push_back(*replacement);
            return VisitAction::WALK_CHILDREN;
        }
        if (node->astKind != ASTKind::REF_EXPR) {
            return VisitAction::WALK_CHILDREN;
        }
        auto refExpr = DynamicCast<Cangjie::AST::RefExpr *>(node.get());
        if (!refExpr) {
            return VisitAction::WALK_CHILDREN;
        }
        if (auto replacement = BuildReferenceReplacement(context, callExpr, paramList, *refExpr)) {
            replacements.push_back(*replacement);
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return replacements;
}

static std::string ApplyCallSiteArgumentReplacements(
    const CallSiteContext &context, std::vector<ArgumentReplacement> replacements)
{
    if (replacements.empty()) {
        return context.argumentText;
    }

    std::sort(replacements.begin(), replacements.end(),
        [](const auto &left, const auto &right) { return left.first.start < right.first.start; });

    std::ostringstream result;
    Position cursor = context.range.start;
    for (const auto &[replaceRange, replaceText] : replacements) {
        result << context.sel.arkAst->sourceManager->GetContentBetween(cursor, replaceRange.start);
        result << replaceText;
        cursor = replaceRange.end;
    }
    result << context.sel.arkAst->sourceManager->GetContentBetween(cursor, context.range.end);
    return result.str();
}

static std::string BuildCallSiteArgumentText(const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr)
{
    if (!context.sel.arkAst || !context.sel.arkAst->sourceManager || !context.sourceFuncDecl.funcBody ||
        context.sourceFuncDecl.funcBody->paramLists.empty() || !context.sourceFuncDecl.funcBody->paramLists.front()) {
        return context.argumentText;
    }
    auto selectedExpr = GetIntroduceParameterExpr(context.sel.selectionTree, context.range);
    if (selectedExpr && IsSupportedCompoundAssignExpr(*selectedExpr)) {
        return BuildCompoundAssignArgumentText(context, callExpr);
    }

    auto paramList = context.sourceFuncDecl.funcBody->paramLists.front().get();
    auto replacements = CollectCallSiteArgumentReplacements(context, callExpr, *paramList);
    std::string argumentText;
    if (!replacements.empty()) {
        argumentText = ApplyCallSiteArgumentReplacements(context, std::move(replacements));
    } else {
        argumentText = ReplaceParameterNamesWithCallArguments(context, callExpr, *paramList);
    }
    return ReplaceThisReceiverAtCallSite(context, callExpr, argumentText);
}

static std::string GetCompoundAssignOperatorText(Cangjie::TokenKind tokenKind)
{
    switch (tokenKind) {
        case TokenKind::EXP_ASSIGN:
            return "**";
        case TokenKind::MUL_ASSIGN:
            return "*";
        case TokenKind::DIV_ASSIGN:
            return "/";
        case TokenKind::ADD_ASSIGN:
            return "+";
        case TokenKind::SUB_ASSIGN:
            return "-";
        case TokenKind::MOD_ASSIGN:
            return "%";
        case TokenKind::LSHIFT_ASSIGN:
            return "<<";
        case TokenKind::RSHIFT_ASSIGN:
            return ">>";
        case TokenKind::AND_ASSIGN:
            return "&&";
        case TokenKind::BITXOR_ASSIGN:
            return "^";
        case TokenKind::BITAND_ASSIGN:
            return "&";
        case TokenKind::BITOR_ASSIGN:
            return "|";
        case TokenKind::OR_ASSIGN:
            return "||";
        default:
            return "";
    }
}

static std::string BuildCompoundAssignArgumentText(const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr)
{
    auto selectedExpr = GetIntroduceParameterExpr(context.sel.selectionTree, context.range);
    auto assignExpr = DynamicCast<Cangjie::AST::AssignExpr *>(selectedExpr.get());
    if (!assignExpr || !assignExpr->leftValue || !assignExpr->rightExpr || !context.sel.arkAst ||
        !context.sel.arkAst->sourceManager || !context.sourceFuncDecl.funcBody ||
        context.sourceFuncDecl.funcBody->paramLists.empty() || !context.sourceFuncDecl.funcBody->paramLists.front()) {
        return context.argumentText;
    }

    auto paramList = context.sourceFuncDecl.funcBody->paramLists.front().get();
    auto replacements = CollectCallSiteArgumentReplacements(context, callExpr, *paramList);
    std::string leftText = ApplyCallSiteArgumentReplacements(context, std::move(replacements));
    std::string rightText = context.sel.arkAst->sourceManager->GetContentBetween(
        assignExpr->rightExpr->begin, assignExpr->rightExpr->end);
    auto opText = GetCompoundAssignOperatorText(assignExpr->op);
    if (leftText.empty() || rightText.empty() || opText.empty()) {
        return context.argumentText;
    }
    return ReplaceThisReceiverAtCallSite(context, callExpr, leftText + " " + opText + " " + rightText);
}

static Ptr<Cangjie::AST::FuncArg> FindCallSiteArgByParam(
    Cangjie::AST::CallExpr &callExpr, Cangjie::AST::FuncParam &param, std::size_t paramIndex)
{
    std::string paramName = GetParamCallName(param);
    for (auto &arg : callExpr.args) {
        if (arg && !arg->name.Empty() && GetIdentifierText(arg->name) == paramName) {
            return arg.get();
        }
    }
    if (param.isNamedParam) {
        return nullptr;
    }
    if (paramIndex < callExpr.args.size()) {
        return callExpr.args[paramIndex].get();
    }
    return nullptr;
}

static std::string GetArgumentTextForParam(
    const CallSiteContext &context,
    Cangjie::AST::CallExpr &callExpr,
    Cangjie::AST::FuncParam &param,
    std::size_t paramIndex)
{
    if (!context.sel.arkAst || !context.sel.arkAst->sourceManager) {
        return "";
    }
    auto arg = FindCallSiteArgByParam(callExpr, param, paramIndex);
    if (arg && arg->expr) {
        return context.sel.arkAst->sourceManager->GetContentBetween(arg->expr->begin, arg->expr->end);
    }
    if (param.assignment) {
        return context.sel.arkAst->sourceManager->GetContentBetween(param.assignment->begin, param.assignment->end);
    }
    return "";
}

static std::string GetIdentifierText(const Cangjie::Identifier &identifier)
{
    return identifier.Val();
}

static std::string GetParamCallName(const Cangjie::AST::FuncParam &param)
{
    std::string paramName = GetIdentifierText(param.identifier);
    if (!paramName.empty() && paramName.back() == '!') {
        paramName.pop_back();
    }
    return paramName;
}

static bool IsIdentifierChar(char ch)
{
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

static std::string ReplaceIdentifierToken(
    const std::string &text, const std::string &identifier, const std::string &replacement)
{
    if (identifier.empty()) {
        return text;
    }
    std::string result;
    std::size_t cursor = 0;
    while (cursor < text.size()) {
        auto pos = text.find(identifier, cursor);
        if (pos == std::string::npos) {
            result.append(text.substr(cursor));
            break;
        }
        bool leftBoundary = pos == 0 || !IsIdentifierChar(text[pos - 1]);
        auto end = pos + identifier.size();
        bool rightBoundary = end >= text.size() || !IsIdentifierChar(text[end]);
        result.append(text.substr(cursor, pos - cursor));
        if (leftBoundary && rightBoundary) {
            result.append(replacement);
        } else {
            result.append(identifier);
        }
        cursor = end;
    }
    return result;
}

static std::string ReplaceParameterNamesWithCallArguments(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr, Cangjie::AST::FuncParamList &paramList)
{
    if (!context.sel.arkAst || !context.sel.arkAst->sourceManager) {
        return context.argumentText;
    }

    std::map<std::string, std::string> paramToArgument;
    for (std::size_t index = 0; index < paramList.params.size(); ++index) {
        if (!paramList.params[index]) {
            continue;
        }
        std::string argText = GetArgumentTextForParam(context, callExpr, *paramList.params[index], index);
        if (argText.empty()) {
            continue;
        }
        paramToArgument[GetParamCallName(*paramList.params[index])] = argText;
    }

    std::string result = context.argumentText;
    for (const auto &[param, argument] : paramToArgument) {
        result = ReplaceIdentifierToken(result, param, argument);
    }
    return result;
}

static std::optional<std::string> GetMemberVariableInitializer(
    const Tweak::Selection &sel, Cangjie::AST::MemberAccess &memberAccess)
{
    if (!sel.arkAst || !sel.arkAst->sourceManager) {
        return std::nullopt;
    }

    auto refExpr = DynamicCast<Cangjie::AST::RefExpr *>(memberAccess.baseExpr.get());
    if (!refExpr || !refExpr->isThis) {
        return std::nullopt;
    }

    if (!memberAccess.target) {
        return std::nullopt;
    }

    auto varDecl = DynamicCast<Cangjie::AST::VarDecl *>(memberAccess.target.get());
    if (!varDecl || !varDecl->initializer) {
        return std::nullopt;
    }

    auto initExpr = varDecl->initializer.get();
    if (!initExpr) {
        return std::nullopt;
    }

    if (initExpr->astKind != ASTKind::LIT_CONST_EXPR) {
        return std::nullopt;
    }

    return sel.arkAst->sourceManager->GetContentBetween(initExpr->begin, initExpr->end);
}

static std::string ReplaceThisReceiverAtCallSite(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr, const std::string &argumentText)
{
    if (!context.sel.arkAst || !context.sel.arkAst->sourceManager) {
        return argumentText;
    }

    std::string receiverText;
    
    if (callExpr.baseFunc && callExpr.baseFunc->astKind == ASTKind::MEMBER_ACCESS) {
        auto memberAccess = DynamicCast<Cangjie::AST::MemberAccess *>(callExpr.baseFunc.get());
        if (memberAccess && memberAccess->baseExpr) {
            receiverText = context.sel.arkAst->sourceManager->GetContentBetween(
                memberAccess->baseExpr->begin, memberAccess->baseExpr->end);
        }
    }

    if (receiverText.empty()) {
        return argumentText;
    }

    std::string result;
    std::size_t cursor = 0;
    const std::string thisPrefix = "this.";
    while (cursor < argumentText.size()) {
        auto pos = argumentText.find(thisPrefix, cursor);
        if (pos == std::string::npos) {
            result.append(argumentText.substr(cursor));
            break;
        }
        bool leftBoundary = pos == 0 || !IsIdentifierChar(argumentText[pos - 1]);
        result.append(argumentText.substr(cursor, pos - cursor));
        if (leftBoundary) {
            result.append(receiverText).append(".");
        } else {
            result.append(thisPrefix);
        }
        cursor = pos + thisPrefix.size();
    }
    return result;
}
// LCOV_EXCL_BR_STOP
} // namespace ark
