// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <cangjie/AST/Walker.h>
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
static void UpdateCallSites(const CallSiteContext &context, std::map<std::string, std::vector<TextEdit>> &applyEdits);
static std::optional<TextEdit> InsertArgumentAtCallSite(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr);
static std::string BuildCallSiteArgumentText(const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr);
static Ptr<Cangjie::AST::FuncArg> FindCallSiteArgByParam(
    Cangjie::AST::CallExpr &callExpr, Cangjie::AST::FuncParam &param, std::size_t paramIndex);
static bool ShouldIntroduceNamedParameter(Cangjie::AST::FuncDecl &funcDecl);
static std::string BuildNewParamText(
    Cangjie::AST::FuncDecl &funcDecl, const std::string &paramName, const std::string &typeName);

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

static Ptr<Cangjie::AST::Expr> GetContainingBinaryExpr(
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
        if (treeNode->node->astKind == ASTKind::BINARY_EXPR && treeNode->node->end == selectedRange.end &&
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
        auto binaryExpr = GetContainingBinaryExpr(selectionTree, selectedRange);
        if (!binaryExpr || !binaryExpr->GetTy() || GetString(*binaryExpr->GetTy()) == "UnknownType") {
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
    return GetContainingBinaryExpr(selectionTree, selectedRange);
}

static std::string GetIntroduceParameterExprTypeName(const SelectionTree &selectionTree,
    const Range &selectedRange)
{
    auto exactTypeName = TweakUtils::GetSelectedExprTypeName(selectionTree, selectedRange);
    if (!exactTypeName.empty()) {
        return exactTypeName;
    }
    auto expr = GetIntroduceParameterExpr(selectionTree, selectedRange);
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
                std::to_string(static_cast<int>(IntroduceParameter::IntroduceParameterError::INVALID_CODE_SEGMENT))));
            return false;
        }
        auto funcDecl = IntroduceParameter::GetTargetFunc(sel);
        if (!funcDecl) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceParameter::IntroduceParameterError::INVALID_SCOPE))));
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
    std::string argumentText = sel.arkAst->sourceManager->GetContentBetween(range.start, range.end);
    if (!HasTargetFunctionLocalVariableReference(sel, *funcDecl) && CanBuildArgumentAtCallSites(sel, *funcDecl)) {
        CallSiteContext callSiteContext{sel, *funcDecl, range, argumentText, paramName, removedParamIndices};
        UpdateCallSites(callSiteContext, effect.applyEdits);
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

TextEdit IntroduceParameter::ReplaceExprWithParam(const Selection &sel, Range &range, std::string &paramName)
{
    TextEdit textEdit;
    textEdit.newText = paramName;
    std::string charContent = sel.arkAst->sourceManager->GetContentBetween(
        {range.start.fileID, range.start.line, 1}, range.start);
    int size = range.end.column - range.start.column;
    range.start.column = CountUnicodeCharacters(charContent) + 1;
    if (range.end.line == range.start.line) {
        range.end.column = range.start.column + size;
    }
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

static bool HasNamedArg(Cangjie::AST::CallExpr &callExpr)
{
    for (const auto &arg : callExpr.args) {
        if (arg && !arg->name.Empty()) {
            return true;
        }
    }
    return false;
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
    std::vector<Range> argRanges;
    for (auto index : context.removedParamIndices) {
        if (index >= paramList->params.size() || !paramList->params[index]) {
            return std::nullopt;
        }
        auto arg = FindCallSiteArgByParam(callExpr, *paramList->params[index], index);
        if (!arg) {
            return std::nullopt;
        }
        argRanges.push_back({arg->begin, arg->end});
    }
    std::sort(argRanges.begin(), argRanges.end(),
        [](const auto &left, const auto &right) { return left.start < right.start; });

    TextEdit textEdit;
    textEdit.range = TransformFromChar2IDE({argRanges.front().start, argRanges.back().end});
    textEdit.newText = hasNamedArg ? context.paramName + ": " + newArgument : newArgument;
    return textEdit;
// LCOV_EXCL_BR_STOP
}

// LCOV_EXCL_START
static TextEdit InsertNewCallArgument(
    const CallSiteContext &context, Cangjie::AST::CallExpr &callExpr, const std::string &newArgument, bool hasNamedArg)
{
    TextEdit textEdit;
    bool introduceNamed = ShouldIntroduceNamedParameter(context.funcDecl);
    std::ostringstream insertText;
    textEdit.range = TransformFromChar2IDE({callExpr.rightParenPos, callExpr.rightParenPos});
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
        auto arg = FindCallSiteArgByParam(callExpr, *paramList.params[index], index);
        if (!arg || !arg->expr) {
            return std::nullopt;
        }
        Range refRange = {refExpr.begin, refExpr.end};
        if (refRange.start < context.range.start || refRange.end > context.range.end) {
            return std::nullopt;
        }
        auto argExpr = arg->expr.get();
        std::string argText =
            context.sel.arkAst->sourceManager->GetContentBetween(argExpr->begin, argExpr->end);
        return ArgumentReplacement{refRange, argText};
    }
    return std::nullopt;
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
        if (node->begin < context.range.start || node->end > context.range.end) {
            return VisitAction::SKIP_CHILDREN;
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
    if (!context.sel.arkAst || !context.sel.arkAst->sourceManager || !context.funcDecl.funcBody ||
        context.funcDecl.funcBody->paramLists.empty() || !context.funcDecl.funcBody->paramLists.front()) {
        return context.argumentText;
    }

    auto paramList = context.funcDecl.funcBody->paramLists.front().get();
    auto replacements = CollectCallSiteArgumentReplacements(context, callExpr, *paramList);
    return ApplyCallSiteArgumentReplacements(context, std::move(replacements));
}

static Ptr<Cangjie::AST::FuncArg> FindCallSiteArgByParam(
    Cangjie::AST::CallExpr &callExpr, Cangjie::AST::FuncParam &param, std::size_t paramIndex)
{
    for (auto &arg : callExpr.args) {
        if (arg && !arg->name.Empty() && arg->name == param.identifier) {
            return arg.get();
        }
    }
    if (paramIndex < callExpr.args.size()) {
        return callExpr.args[paramIndex].get();
    }
    return nullptr;
}
// LCOV_EXCL_BR_STOP
} // namespace ark
