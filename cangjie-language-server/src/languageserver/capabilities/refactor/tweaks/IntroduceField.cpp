// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <cangjie/AST/Walker.h>
#include "../../../common/Utils.h"
#include "../TweakRule.h"
#include "../TweakUtils.h"
#include "IntroduceField.h"

namespace ark {
const std::unordered_set<Cangjie::AST::ASTKind> CANNOT_INTRODUCE_FIELD_EXPR = {
    ASTKind::BLOCK, ASTKind::STR_INTERPOLATION_EXPR, ASTKind::INTERPOLATION_EXPR
};
constexpr int BLANK_LINE_NEWLINE_COUNT = 2;
struct LocalVarTarget {
    Ptr<Cangjie::AST::VarDecl> decl = nullptr;
    Range declarationRange;
};

struct MemberInitializerTarget {
    Ptr<Cangjie::AST::VarDecl> decl = nullptr;
    Ptr<Cangjie::AST::InheritableDecl> owner = nullptr;
    bool isGlobal = false;
};

static TextEdit InsertDefaultFieldDeclaration(const Tweak::Selection &sel, Range &range, std::string &fieldName,
    std::string &typeName, Cangjie::AST::FuncDecl &funcDecl);
static TextEdit BuildLocalVarDeclarationReplacement(const Tweak::Selection &sel, Cangjie::AST::VarDecl &decl,
    const std::string &fieldName, Cangjie::AST::FuncDecl &funcDecl);
static std::vector<TextEdit> BuildLocalVarReferenceReplacements(Cangjie::AST::FuncDecl &funcDecl,
    Cangjie::AST::VarDecl &decl, const std::string &fieldName);

static std::string GetDefaultInitializer(const Tweak::Selection &sel, const Range &range, const std::string &typeName);
static std::string GetIntroduceFieldTypeName(const SelectionTree &selectionTree, const Range &range);
static Range GetIntroduceFieldExprRange(const SelectionTree &selectionTree, const Range &selectedRange);
static Ptr<Cangjie::AST::Expr> GetIntroduceFieldExpr(const SelectionTree &selectionTree, const Range &selectedRange);
static std::optional<MemberInitializerTarget> GetMemberInitializerTarget(const Tweak::Selection &sel,
    const Range &range);
static std::optional<Tweak::Effect> BuildMemberInitializerEffect(const Tweak::Selection &sel, Range &range,
    const std::string &fieldName, const std::string &typeName, const MemberInitializerTarget &target);

static bool CanUseAsFieldInitializer(const Tweak::Selection &sel, const Range &range, Cangjie::AST::FuncDecl &funcDecl)
{
    (void)funcDecl;
    auto expr = GetIntroduceFieldExpr(sel.selectionTree, range);
    return expr && expr->astKind == ASTKind::LIT_CONST_EXPR && !IntroduceField::IsStringInterpolationLiteral(expr);
}

static std::string GetDefaultInitializer(const std::string &typeName)
{
    static const std::unordered_map<std::string, std::string> DEFAULT_INITIALIZERS = {
        {"Float64", "0.0"},
        {"Float32", "0.0"},
        {"Float16", "0.0"},
        {"Bool", "false"},
        {"String", "\"\""},
        {"Rune", "r'\\0'"},
        {"Unit", "()"},
        {"Int8", "0"},
        {"Int16", "0"},
        {"Int32", "0"},
        {"Int64", "0"},
        {"IntNative", "0"},
        {"UInt8", "0"},
        {"UInt16", "0"},
        {"UInt32", "0"},
        {"UInt64", "0"},
        {"UIntNative", "0"},
    };

    auto it = DEFAULT_INITIALIZERS.find(typeName);
    return it != DEFAULT_INITIALIZERS.end() ? it->second : "";
}

static bool IsConstInitializerSelection(const Tweak::Selection &sel, const Range &range)
{
    bool isConstInitializer = false;
    if (!sel.arkAst || !sel.arkAst->file) {
        return false;
    }
    ConstWalker(sel.arkAst->file, [&range, &isConstInitializer](Ptr<const Cangjie::AST::Node> node) {
        if (!node || isConstInitializer) {
            return VisitAction::STOP_NOW;
        }
        auto varDecl = DynamicCast<const Cangjie::AST::VarDeclAbstract *>(node.get());
        if (!varDecl || !varDecl->isConst || !varDecl->initializer) {
            return VisitAction::WALK_CHILDREN;
        }
        if (varDecl->initializer->begin <= range.start && varDecl->initializer->end >= range.end) {
            isConstInitializer = true;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return isConstInitializer;
}

static bool IsLetPatternDestructorSelection(const SelectionTree &selectionTree, const Range &range)
{
    auto root = selectionTree.root();
    if (!root || !root->node) {
        return false;
    }
    bool isLetPatternDestructor = false;
    SelectionTree::Walk(root, [&range, &isLetPatternDestructor](const SelectionTree::SelectionTreeNode *node) {
        if (!node || !node->node || isLetPatternDestructor) {
            return SelectionTree::WalkAction::STOP_NOW;
        }
        if (node->selected == SelectionTree::Selection::Complete &&
            node->node->astKind == ASTKind::LET_PATTERN_DESTRUCTOR &&
            node->node->begin == range.start && node->node->end == range.end) {
            isLetPatternDestructor = true;
            return SelectionTree::WalkAction::STOP_NOW;
        }
        return SelectionTree::WalkAction::WALK_CHILDREN;
    });
    return isLetPatternDestructor;
}

static bool IsSelectionInLambdaExpr(const Tweak::Selection &sel, const Range &range)
{
    bool isInLambda = false;
    if (!sel.arkAst || !sel.arkAst->file) {
        return false;
    }
    ConstWalker(sel.arkAst->file, [&range, &isInLambda](Ptr<const Cangjie::AST::Node> node) {
        if (!node || isInLambda) {
            return VisitAction::STOP_NOW;
        }
        if (node->begin > range.start || node->end < range.end) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind == ASTKind::LAMBDA_EXPR) {
            isInLambda = true;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return isInLambda;
}

static Ptr<Cangjie::AST::RefExpr> GetSelectedRefExpr(const SelectionTree &selectionTree, const Range &range)
{
    Ptr<Cangjie::AST::RefExpr> selectedRef = nullptr;
    auto root = selectionTree.root();
    if (!root || !root->node) {
        return nullptr;
    }
    SelectionTree::Walk(root, [&range, &selectedRef](const SelectionTree::SelectionTreeNode *node) {
        if (!node || !node->node) {
            return SelectionTree::WalkAction::STOP_NOW;
        }
        if (node->selected == SelectionTree::Selection::Complete && node->node->astKind == ASTKind::REF_EXPR &&
            node->node->begin == range.start && node->node->end == range.end) {
            selectedRef = DynamicCast<Cangjie::AST::RefExpr *>(node->node.get());
            return SelectionTree::WalkAction::STOP_NOW;
        }
        return SelectionTree::WalkAction::WALK_CHILDREN;
    });
    return selectedRef;
}

static Ptr<Cangjie::AST::Expr> GetSelectedExpr(const SelectionTree &selectionTree, const Range &range)
{
    Ptr<Cangjie::AST::Expr> selectedExpr = nullptr;
    auto root = selectionTree.root();
    if (!root || !root->node) {
        return nullptr;
    }
    SelectionTree::Walk(root, [&range, &selectedExpr](const SelectionTree::SelectionTreeNode *node) {
        if (!node || !node->node) {
            return SelectionTree::WalkAction::STOP_NOW;
        }
        if (node->selected == SelectionTree::Selection::Complete && node->node->IsExpr() &&
            node->node->begin == range.start && node->node->end == range.end) {
            selectedExpr = DynamicCast<Cangjie::AST::Expr *>(node->node.get());
            return SelectionTree::WalkAction::STOP_NOW;
        }
        return SelectionTree::WalkAction::WALK_CHILDREN;
    });
    return selectedExpr;
}

static Range GetIntroduceFieldExprRange(const SelectionTree &selectionTree, const Range &selectedRange)
{
    auto exprNode = TweakUtils::GetCompleteExprNode(selectionTree, selectedRange);
    if (exprNode && exprNode->node) {
        return {exprNode->node->begin, exprNode->node->end};
    }
    return TweakUtils::GetCompleteExprRange(selectionTree);
}

static Ptr<Cangjie::AST::Expr> GetIntroduceFieldExpr(const SelectionTree &selectionTree, const Range &selectedRange)
{
    auto exprNode = TweakUtils::GetCompleteExprNode(selectionTree, selectedRange);
    if (exprNode && exprNode->node) {
        return DynamicCast<Cangjie::AST::Expr *>(exprNode->node.get());
    }
    return GetSelectedExpr(selectionTree, TweakUtils::GetCompleteExprRange(selectionTree));
}

static bool IsLocalVarDecl(Cangjie::AST::VarDecl &decl)
{
    return !decl.TestAnyAttr(Attribute::GLOBAL, Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM);
}

static bool IsFieldLikeVarDecl(const Cangjie::AST::VarDecl &decl)
{
    return decl.TestAnyAttr(Attribute::GLOBAL, Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM);
}

static bool IsClassOrStructDecl(const Cangjie::AST::Decl &decl)
{
    return decl.astKind == ASTKind::CLASS_DECL || decl.astKind == ASTKind::STRUCT_DECL;
}
// LCOV_EXCL_BR_START
template <typename BodyDecl>
static Ptr<Cangjie::AST::InheritableDecl> FindOwnerInBody(BodyDecl &decl, const Cangjie::AST::VarDecl &target)
{
    if (!decl.body) {
        return nullptr;
    }
    for (auto &member : decl.body->decls) {
        if (member && member.get() == &target) {
            return DynamicCast<Cangjie::AST::InheritableDecl *>(&decl);
        }
    }
    return nullptr;
}
// LCOV_EXCL_BR_STOP
static Ptr<Cangjie::AST::InheritableDecl> FindMemberOwner(const Tweak::Selection &sel,
    const Cangjie::AST::VarDecl &decl)
{
    auto outerDecl = decl.outerDecl ? DynamicCast<Cangjie::AST::InheritableDecl *>(decl.outerDecl.get()) : nullptr;
    if (outerDecl && IsClassOrStructDecl(*outerDecl)) {
        return outerDecl;
    }
    if (!sel.arkAst || !sel.arkAst->file) {
        return nullptr;
    }
    for (auto &topLevelDecl : sel.arkAst->file->decls) {
        if (!topLevelDecl) {
            continue;
        }
        if (auto classDecl = DynamicCast<Cangjie::AST::ClassDecl *>(topLevelDecl.get())) {
            if (auto owner = FindOwnerInBody(*classDecl, decl)) {
                return owner;
            }
        } else if (auto structDecl = DynamicCast<Cangjie::AST::StructDecl *>(topLevelDecl.get())) {
            if (auto owner = FindOwnerInBody(*structDecl, decl)) {
                return owner;
            }
        }
    }
    return nullptr;
}

static bool IsMemberInitializerSelection(const Cangjie::AST::VarDecl &decl, const Range &range)
{
    return decl.initializer && decl.initializer->begin <= range.start && decl.initializer->end >= range.end;
}

static bool IsExistingFieldDeclarationSelection(const Tweak::Selection &sel, const Range &range)
{
    bool isFieldDeclaration = false;
    if (!sel.arkAst || !sel.arkAst->file) {
        return false;
    }
    ConstWalker(sel.arkAst->file, [&sel, &range, &isFieldDeclaration](Ptr<const Cangjie::AST::Node> node) {
        if (!node || isFieldDeclaration) {
            return VisitAction::STOP_NOW;
        }
        auto varDecl = DynamicCast<const Cangjie::AST::VarDecl *>(node.get());
        if (!varDecl || !IsFieldLikeVarDecl(*varDecl)) {
            return VisitAction::WALK_CHILDREN;
        }
        if (IsMemberInitializerSelection(*varDecl, range)) {
            return VisitAction::WALK_CHILDREN;
        }
        if (varDecl->begin <= sel.range.start && varDecl->end >= sel.range.end) {
            isFieldDeclaration = true;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return isFieldDeclaration;
}

static std::optional<LocalVarTarget> GetLocalVarTarget(
    const SelectionTree &selectionTree, const Range &range, Cangjie::AST::FuncDecl &funcDecl)
{
    auto selectedRef = GetSelectedRefExpr(selectionTree, range);
    auto targetDecl = selectedRef ? DynamicCast<Cangjie::AST::VarDecl *>(selectedRef->GetTarget().get()) : nullptr;
// LCOV_EXCL_BR_START
    if (!targetDecl || !IsLocalVarDecl(*targetDecl) || targetDecl->isConst ||
        !funcDecl.funcBody || !funcDecl.funcBody->body ||
        targetDecl->begin < funcDecl.funcBody->body->begin || targetDecl->end > funcDecl.funcBody->body->end) {
        return std::nullopt;
    }
// LCOV_EXCL_BR_STOP
    LocalVarTarget target;
    target.decl = targetDecl;
    target.declarationRange = {targetDecl->begin, targetDecl->end};
    return target;
}

static std::optional<MemberInitializerTarget> GetMemberInitializerTarget(const Tweak::Selection &sel,
    const Range &range)
{
    auto selectedDecl = DynamicCast<Cangjie::AST::VarDecl *>(sel.selectionTree.TargetDecl().get());
    if (selectedDecl && IsFieldLikeVarDecl(*selectedDecl) && IsMemberInitializerSelection(*selectedDecl, range)) {
        if (selectedDecl->TestAttr(Attribute::GLOBAL)) {
            return MemberInitializerTarget{selectedDecl, nullptr, true};
        }
        auto owner = FindMemberOwner(sel, *selectedDecl);
        if (owner && IsClassOrStructDecl(*owner)) {
            return MemberInitializerTarget{selectedDecl, owner, false};
        }
    }
    if (!sel.arkAst || !sel.arkAst->file) {
        return std::nullopt;
    }
    MemberInitializerTarget target;
    ConstWalker(sel.arkAst->file, [&sel, &range, &target](Ptr<const Cangjie::AST::Node> node) {
        if (!node || target.decl) {
            return VisitAction::STOP_NOW;
        }
        auto varDecl = DynamicCast<const Cangjie::AST::VarDecl *>(node.get());
        if (!varDecl || !IsFieldLikeVarDecl(*varDecl) || !IsMemberInitializerSelection(*varDecl, range)) {
            return VisitAction::WALK_CHILDREN;
        }
        auto mutableDecl = const_cast<Cangjie::AST::VarDecl *>(varDecl);
        if (mutableDecl->TestAttr(Attribute::GLOBAL)) {
            target.decl = mutableDecl;
            target.isGlobal = true;
            return VisitAction::STOP_NOW;
        }
        auto owner = FindMemberOwner(sel, *mutableDecl);
        if (!owner || !IsClassOrStructDecl(*owner)) {
            return VisitAction::WALK_CHILDREN;
        }
        target.decl = mutableDecl;
        target.owner = owner;
        target.isGlobal = false;
        return VisitAction::STOP_NOW;
    }).Walk();
    if (!target.decl || (!target.owner && !target.isGlobal)) {
        return std::nullopt;
    }
    return target;
}

static Ptr<Cangjie::AST::EnumDecl> GetEnumDeclFromTy(Ptr<Cangjie::AST::Ty> ty)
{
    if (!ty) {
        return nullptr;
    }
    if (auto enumTy = dynamic_cast<Cangjie::AST::EnumTy *>(ty.get())) {
        return enumTy->decl ? enumTy->decl : enumTy->declPtr;
    }
    if (auto aliasTy = dynamic_cast<Cangjie::AST::TypeAliasTy *>(ty.get())) {
        if (!aliasTy->declPtr || !aliasTy->declPtr->type) {
            return nullptr;
        }
        auto target = aliasTy->declPtr->type->GetTarget();
        if (auto enumDecl = DynamicCast<Cangjie::AST::EnumDecl *>(target.get())) {
            return enumDecl;
        }
        return DynamicCast<Cangjie::AST::EnumDecl *>(Cangjie::AST::Ty::GetDeclPtrOfTy(
            aliasTy->declPtr->type->GetTy()).get());
    }
    return DynamicCast<Cangjie::AST::EnumDecl *>(Cangjie::AST::Ty::GetDeclPtrOfTy(ty).get());
}

static std::string GetDeclaredTypeNameForSelectedReference(const Tweak::Selection &sel, const Range &range)
{
    auto selectedRef = GetSelectedRefExpr(sel.selectionTree, range);
    auto varDecl = selectedRef ? DynamicCast<Cangjie::AST::VarDeclAbstract *>(selectedRef->GetTarget().get()) : nullptr;
    if (!varDecl || !varDecl->type) {
        return "";
    }
    if (sel.arkAst && sel.arkAst->sourceManager && varDecl->type->begin < varDecl->type->end) {
        return sel.arkAst->sourceManager->GetContentBetween(varDecl->type->begin, varDecl->type->end);
    }
    return varDecl->type->ToString();
}

static std::string GetEnumDefaultInitializer(const Tweak::Selection &sel, const Range &range)
{
    auto selectedExpr = GetSelectedExpr(sel.selectionTree, range);
    if (!selectedExpr || !selectedExpr->GetTy()) {
        return "";
    }
    auto enumDecl = GetEnumDeclFromTy(selectedExpr->GetTy());
    if (!enumDecl || enumDecl->constructors.empty() || !enumDecl->constructors.front()) {
        return "";
    }
    std::string qualifier = GetDeclaredTypeNameForSelectedReference(sel, range);
    if (qualifier.empty()) {
        qualifier = enumDecl->identifier.Val();
    }
    return qualifier + "." + enumDecl->constructors.front()->identifier.Val();
}

static std::string GetDefaultInitializer(const Tweak::Selection &sel, const Range &range, const std::string &typeName)
{
    std::string initializer = GetDefaultInitializer(typeName);
    if (!initializer.empty()) {
        return initializer;
    }
    return GetEnumDefaultInitializer(sel, range);
}

static std::string BuildTupleTypeName(Cangjie::AST::TupleTy &tupleTy)
{
    std::ostringstream typeName;
    typeName << "(";
    for (size_t i = 0; i < tupleTy.typeArgs.size(); ++i) {
        if (i > 0) {
            typeName << ", ";
        }
        typeName << GetString(*tupleTy.typeArgs[i]);
    }
    typeName << ")";
    return typeName.str();
}

static std::string GetIntroduceFieldTypeName(const SelectionTree &selectionTree, const Range &range)
{
    auto selectedExpr = GetIntroduceFieldExpr(selectionTree, range);
    auto tupleTy = selectedExpr && selectedExpr->GetTy() ?
        dynamic_cast<Cangjie::AST::TupleTy *>(selectedExpr->GetTy().get()) : nullptr;
    if (tupleTy) {
        return BuildTupleTypeName(*tupleTy);
    }
    return TweakUtils::GetSelectedExprTypeName(selectionTree, range);
}

static std::string GetExplicitTypeName(const Tweak::Selection &sel)
{
    auto it = sel.extraOptions.find("typeName");
    return it == sel.extraOptions.end() ? "" : it->second;
}

namespace {
// NOLINTNEXTLINE(G.NAM.02-CPP)
class IntroduceFieldRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (!root || !root->node) {
            return false;
        }
        auto range = GetIntroduceFieldExprRange(sel.selectionTree, sel.range);
        auto selectedExpr = GetIntroduceFieldExpr(sel.selectionTree, sel.range);
        if (range.start == Position{0, 0, 0} || range.start == range.end || !selectedExpr) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::FAIL_GET_ROOT_EXPR))));
            return false;
        }
        auto memberInitializerTarget = GetMemberInitializerTarget(sel, range);
        if (((sel.selectionTree.SelectedScope() & SelectionTree::Scope::GLOBAL_VAR) != SelectionTree::Scope::UNKNOWN &&
            !memberInitializerTarget) ||
            ((sel.selectionTree.SelectedScope() & SelectionTree::Scope::MEMBER_VAR) != SelectionTree::Scope::UNKNOWN &&
            !memberInitializerTarget) || IsExistingFieldDeclarationSelection(sel, range)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::INVALID_SCOPE))));
            return false;
        }
        if (CANNOT_INTRODUCE_FIELD_EXPR.count(selectedExpr->astKind)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::INVALID_CODE_SEGMENT))));
            return false;
        }
        if (TweakUtils::IsSupportedCompoundAssignExpr(*selectedExpr)) {
            extraOptions.insert(std::make_pair("ErrorCode", std::to_string(static_cast<int>(
                IntroduceField::IntroduceFieldError::INVALID_COMPOUND_ASSIGNMENT))));
            return false;
        }
        auto funcDecl = IntroduceField::GetTargetFunc(sel);
        if (!memberInitializerTarget && (!funcDecl || !IntroduceField::IsSupportedTargetFunc(*funcDecl))) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::INVALID_SCOPE))));
            return false;
        }
        if (IsLetPatternDestructorSelection(sel.selectionTree, range)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(
                    IntroduceField::IntroduceFieldError::INVALID_LET_PATTERN_DESTRUCTOR))));
            return false;
        }
        if (IsConstInitializerSelection(sel, range)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::INVALID_CONST_INITIALIZER))));
            return false;
        }
        std::string typeName = GetIntroduceFieldTypeName(sel.selectionTree, range);
        if (typeName.empty()) {
            typeName = GetExplicitTypeName(sel);
        }
        if (typeName.empty()) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::INVALID_TYPE))));
            return false;
        }
        if (!memberInitializerTarget &&
            IntroduceField::IsImmutableStructMemberFieldAssignment(sel, range, typeName, *funcDecl)) {
            extraOptions.insert(std::make_pair("ErrorCode", std::to_string(static_cast<int>(
                IntroduceField::IntroduceFieldError::INVALID_IMMUTABLE_STRUCT_MEMBER_FIELD_ASSIGNMENT))));
            return false;
        }
        return true;
    }
};
}

bool IntroduceField::Prepare(const Selection &sel)
{
    TweakRuleEngine ruleEngine;
    ruleEngine.AddRule(std::make_unique<IntroduceFieldRule>());
    ruleEngine.CheckRules(sel, extraOptions);
    return true;
}

std::optional<Tweak::Effect> IntroduceField::Apply(const Selection &sel)
{
    Effect effect;
    if (!sel.arkAst || !sel.arkAst->sourceManager || !sel.arkAst->file) {
        return std::nullopt;
    }

    std::string fieldName = "newField";
    auto it = sel.extraOptions.find("suggestName");
    if (it != sel.extraOptions.end()) {
        fieldName = it->second;
    }

    Range range = GetIntroduceFieldExprRange(sel.selectionTree, sel.range);
    std::string typeName = GetIntroduceFieldTypeName(sel.selectionTree, range);
    if (typeName.empty()) {
        typeName = GetExplicitTypeName(sel);
    }
    if (typeName.empty()) {
        return std::nullopt;
    }
    auto memberInitializerTarget = GetMemberInitializerTarget(sel, range);
    if (memberInitializerTarget && memberInitializerTarget->decl &&
        (memberInitializerTarget->owner || memberInitializerTarget->isGlobal)) {
        return BuildMemberInitializerEffect(sel, range, fieldName, typeName, *memberInitializerTarget);
    }

    auto funcDecl = GetTargetFunc(sel);
    if (!funcDecl || !IsSupportedTargetFunc(*funcDecl)) {
        return std::nullopt;
    }
    bool useSelectedExprAsInitializer = CanUseAsFieldInitializer(sel, range, *funcDecl) ||
        GetDefaultInitializer(sel, range, typeName).empty();

    std::vector<TextEdit> textEdits;
    auto localVarTarget = GetLocalVarTarget(sel.selectionTree, range, *funcDecl);
    if (localVarTarget && localVarTarget->decl) {
        textEdits.push_back(InsertDefaultFieldDeclaration(sel, range, fieldName, typeName, *funcDecl));
        textEdits.push_back(BuildLocalVarDeclarationReplacement(sel, *localVarTarget->decl, fieldName, *funcDecl));
        auto referenceEdits = BuildLocalVarReferenceReplacements(*funcDecl, *localVarTarget->decl, fieldName);
        textEdits.insert(textEdits.end(), referenceEdits.begin(), referenceEdits.end());
        std::string uri = URI::URIFromAbsolutePath(sel.arkAst->file->filePath).ToString();
        effect.applyEdits.emplace(uri, std::move(textEdits));
        effect.format = false;
        return std::move(effect);
    }
    textEdits.push_back(InsertFieldDeclaration(sel, range, fieldName, typeName, *funcDecl));
    if (!useSelectedExprAsInitializer && !IsSelectionInLambdaExpr(sel, range)) {
        textEdits.push_back(InsertFieldAssignment(sel, range, fieldName, *funcDecl));
    }
    textEdits.push_back(ReplaceExprWithField(sel, range, fieldName, *funcDecl));

    std::string uri = URI::URIFromAbsolutePath(sel.arkAst->file->filePath).ToString();
    effect.applyEdits.emplace(uri, std::move(textEdits));
    effect.format = false;
    return std::move(effect);
}

std::map<std::string, std::string> IntroduceField::ExtraOptions()
{
    return extraOptions;
}

Ptr<Cangjie::AST::FuncDecl> IntroduceField::GetTargetFunc(const Selection &sel)
{
    return TweakUtils::GetTargetFunc(
        sel.selectionTree, sel.arkAst, TweakUtils::GetCompleteExprRange(sel.selectionTree));
}

bool IntroduceField::IsSupportedTargetFunc(Cangjie::AST::FuncDecl &funcDecl)
{
    if (!funcDecl.outerDecl) {
        return true;
    }
    return funcDecl.outerDecl->astKind == ASTKind::CLASS_DECL || funcDecl.outerDecl->astKind == ASTKind::STRUCT_DECL ||
        funcDecl.outerDecl->astKind == ASTKind::FUNC_DECL;
}

Ptr<Cangjie::AST::InheritableDecl> IntroduceField::GetOwnerDecl(const Selection &sel)
{
    auto funcDecl = GetTargetFunc(sel);
    if (!funcDecl || !funcDecl->outerDecl) {
        return nullptr;
    }
    auto owner = DynamicCast<Cangjie::AST::InheritableDecl *>(funcDecl->outerDecl.get());
    if (!owner || (owner->astKind != ASTKind::CLASS_DECL && owner->astKind != ASTKind::STRUCT_DECL)) {
        return nullptr;
    }
    return owner;
}

bool IntroduceField::IsMemberFieldTarget(Cangjie::AST::FuncDecl &funcDecl)
{
    if (!funcDecl.outerDecl) {
        return false;
    }
    return funcDecl.outerDecl->astKind == ASTKind::CLASS_DECL || funcDecl.outerDecl->astKind == ASTKind::STRUCT_DECL;
}

bool IntroduceField::IsStaticFieldTarget(Cangjie::AST::FuncDecl &funcDecl)
{
    return IsMemberFieldTarget(funcDecl) && funcDecl.TestAttr(Attribute::STATIC);
}

bool IntroduceField::IsStringInterpolationLiteral(Ptr<Cangjie::AST::Expr> expr)
{
    auto litConstExpr = expr ? DynamicCast<Cangjie::AST::LitConstExpr *>(expr.get()) : nullptr;
    return litConstExpr && litConstExpr->siExpr;
}

bool IntroduceField::IsImmutableStructMemberFieldAssignment(
    const Selection &sel, Range &range, const std::string &typeName, Cangjie::AST::FuncDecl &funcDecl)
{
    if (!funcDecl.outerDecl || funcDecl.outerDecl->astKind != ASTKind::STRUCT_DECL ||
        funcDecl.TestAttr(Attribute::MUT) || IsStaticFieldTarget(funcDecl)) {
        return false;
    }
    return !CanUseAsFieldInitializer(sel, range, funcDecl) && !GetDefaultInitializer(sel, range, typeName).empty();
}

static Position GetDeclarationInsertStart(const Cangjie::AST::Decl &decl)
{
    return {decl.begin.fileID, decl.begin.line, 1};
}

static Position GetDeclarationInsertEnd(const Cangjie::AST::Decl &decl)
{
    return decl.end;
}

static Position GetCodeBegin(const Tweak::Selection &sel, const Cangjie::AST::Node &node)
{
    if (!sel.arkAst) {
        return node.begin;
    }
    for (const auto &token : sel.arkAst->tokens) {
        if (token.Begin() < node.begin) {
            continue;
        }
        if (token.Begin() > node.end) {
            break;
        }
        return token.Begin();
    }
    return node.begin;
}

static bool IsFieldDecl(const Cangjie::AST::Decl &decl)
{
    return decl.astKind == ASTKind::VAR_DECL;
}

static Position GetLeadingCommentStart(
    const Tweak::Selection &sel, const Position &searchStart, const Position &codeBegin)
{
// LCOV_EXCL_BR_START
    if (!sel.arkAst || !sel.arkAst->sourceManager || searchStart >= codeBegin) {
        return codeBegin;
    }
    std::string prefix = sel.arkAst->sourceManager->GetContentBetween(searchStart, codeBegin);
    size_t offset = prefix.size();
    size_t commentStart = std::string::npos;
    while (offset > 0) {
        size_t lineStart = prefix.rfind('\n', offset - 1);
        lineStart = lineStart == std::string::npos ? 0 : lineStart + 1;
        std::string line = prefix.substr(lineStart, offset - lineStart);
        size_t firstNonSpace = line.find_first_not_of(" \t\r");
        if (firstNonSpace == std::string::npos) {
            offset = lineStart == 0 ? 0 : lineStart - 1;
            continue;
        }
        bool isLineComment = line.compare(firstNonSpace, 2, "//") == 0;
        bool isBlockCommentEnd = line.compare(firstNonSpace, 2, "*/") == 0 ||
            line.find("*/", firstNonSpace) != std::string::npos;
        bool isBlockCommentLine = line.compare(firstNonSpace, 2, "/*") == 0 ||
            line.compare(firstNonSpace, 1, "*") == 0;
        if (!isLineComment && !isBlockCommentLine && !isBlockCommentEnd) {
            break;
        }
        commentStart = lineStart;
        offset = lineStart == 0 ? 0 : lineStart - 1;
    }
    return commentStart == std::string::npos ? codeBegin :
        TweakUtils::PositionAtOffset(searchStart, prefix, commentStart);
// LCOV_EXCL_BR_STOP
}

static Position GetDeclInsertStart(
    const Tweak::Selection &sel, const Position &searchStart, Cangjie::AST::Decl &decl)
{
    Position codeBegin = GetCodeBegin(sel, decl);
    return GetLeadingCommentStart(sel, searchStart, codeBegin);
}

template <typename BodyDecl>
static Position GetBodyFirstLineInsertStart(const BodyDecl &decl)
{
    Position insertPos = decl.body->leftCurlPos;
    insertPos.line += 1;
    insertPos.column = 1;
    return insertPos;
}
// LCOV_EXCL_BR_START
template <typename BodyDecl>
static Position GetFieldInsertStartInBody(const BodyDecl &decl, const Cangjie::AST::FuncDecl &funcDecl)
{
    Ptr<Cangjie::AST::Decl> lastField = nullptr;
    for (auto &member : decl.body->decls) {
        if (member && IsFieldDecl(*member) && member->begin < funcDecl.begin) {
            lastField = member.get();
        }
    }
    if (lastField) {
        return GetDeclarationInsertEnd(*lastField);
    }
    return GetBodyFirstLineInsertStart(decl);
}

template <typename BodyDecl>
static Position GetFirstNonFieldLineStartAfterInsertPos(const BodyDecl &decl, const Position &insertPos)
{
    Position firstNonField;
    for (auto &member : decl.body->decls) {
        if (!member || IsFieldDecl(*member) || member->begin <= insertPos) {
            continue;
        }
        if (firstNonField.IsZero() || member->begin < firstNonField) {
            firstNonField = {member->begin.fileID, member->begin.line, 1};
        }
    }
    return firstNonField;
}
// LCOV_EXCL_BR_STOP
static Position GetFieldBlockEndLineStart(Cangjie::AST::FuncDecl &funcDecl, const Position &insertPos)
{
    auto owner = funcDecl.outerDecl ? DynamicCast<Cangjie::AST::InheritableDecl *>(funcDecl.outerDecl.get()) : nullptr;
    if (!owner) {
        return {};
    }
    if (auto classDecl = DynamicCast<Cangjie::AST::ClassDecl *>(owner)) {
        return classDecl->body ? GetFirstNonFieldLineStartAfterInsertPos(*classDecl, insertPos) : Position{};
    }
    if (auto structDecl = DynamicCast<Cangjie::AST::StructDecl *>(owner)) {
        return structDecl->body ? GetFirstNonFieldLineStartAfterInsertPos(*structDecl, insertPos) : Position{};
    }
    return {};
}

static Position GetTopLevelInsertStart(const Tweak::Selection &sel, Cangjie::AST::FuncDecl &funcDecl)
{
    if (!sel.arkAst || !sel.arkAst->file) {
        return GetDeclarationInsertStart(funcDecl);
    }
    auto *topLevelDecl = DynamicCast<Cangjie::AST::Decl *>(&funcDecl);
    while (topLevelDecl && topLevelDecl->outerDecl) {
        topLevelDecl = topLevelDecl->outerDecl.get();
    }
    auto referenceDecl = topLevelDecl ? topLevelDecl : DynamicCast<Cangjie::AST::Decl *>(&funcDecl);
    if (!referenceDecl) {
        return GetDeclarationInsertStart(funcDecl);
    }
    Position previousDeclEnd = sel.arkAst->file->package ? sel.arkAst->file->package->GetEnd() :
        Position{referenceDecl->begin.fileID, 1, 1};
    for (auto &decl : sel.arkAst->file->decls) {
        if (!decl || decl->begin >= referenceDecl->begin) {
            break;
        }
        previousDeclEnd = decl->end;
    }
    return GetDeclInsertStart(sel, previousDeclEnd, *referenceDecl);
}

static Position GetOwnerInsertStart(
    const Tweak::Selection &sel, Cangjie::AST::InheritableDecl &owner, Cangjie::AST::FuncDecl &funcDecl)
{
// LCOV_EXCL_BR_START
    if (auto classDecl = DynamicCast<Cangjie::AST::ClassDecl *>(&owner)) {
        if (!classDecl->body) {
            return {};
        }
        (void)sel;
        return GetFieldInsertStartInBody(*classDecl, funcDecl);
    }
    if (auto structDecl = DynamicCast<Cangjie::AST::StructDecl *>(&owner)) {
        if (!structDecl->body) {
            return {};
        }
        (void)sel;
        return GetFieldInsertStartInBody(*structDecl, funcDecl);
    }
    return {};
// LCOV_EXCL_BR_STOP
}

static bool IsAvailableInitializerDecl(const Cangjie::AST::Decl &decl, bool isMemberField)
{
    if (decl.astKind != ASTKind::VAR_DECL) {
        return false;
    }
    if (isMemberField) {
        return decl.TestAnyAttr(Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM);
    }
    return decl.TestAttr(Attribute::GLOBAL);
}

static Position GetLastDependencyEnd(const Tweak::Selection &sel, const Range &range, bool isMemberField)
{
// LCOV_EXCL_BR_START
    Position dependencyEnd;
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return dependencyEnd;
    }

    Walker(root->node.get(), [&range, &dependencyEnd, isMemberField](auto node) {
        if (!node) {
            return VisitAction::STOP_NOW;
        }
        if (node->begin < range.start || node->end > range.end) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind != ASTKind::REF_EXPR) {
            return VisitAction::WALK_CHILDREN;
        }
        auto refExpr = DynamicCast<Cangjie::AST::RefExpr *>(node.get());
        auto target = refExpr ? DynamicCast<Cangjie::AST::Decl *>(refExpr->GetTarget().get()) : nullptr;
        if (!target || !IsAvailableInitializerDecl(*target, isMemberField)) {
            return VisitAction::WALK_CHILDREN;
        }
        if (dependencyEnd.IsZero() || dependencyEnd < target->end) {
            dependencyEnd = target->end;
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return dependencyEnd;
// LCOV_EXCL_BR_STOP
}

std::optional<Range> IntroduceField::GetFieldInsertRange(
    const Selection &sel, Range &range, Cangjie::AST::FuncDecl &funcDecl)
{
    Position insertPos;
    bool isMemberField = IsMemberFieldTarget(funcDecl);
    if (isMemberField && funcDecl.outerDecl) {
        auto owner = DynamicCast<Cangjie::AST::InheritableDecl *>(funcDecl.outerDecl.get());
        if (!owner) {
            return std::nullopt;
        }
        insertPos = GetOwnerInsertStart(sel, *owner, funcDecl);
        if (insertPos.IsZero()) {
            return std::nullopt;
        }
    } else if (sel.arkAst && sel.arkAst->file) {
        insertPos = GetTopLevelInsertStart(sel, funcDecl);
    } else {
        return std::nullopt;
    }

    Position dependencyEnd = GetLastDependencyEnd(sel, range, isMemberField);
    if (!isMemberField && !dependencyEnd.IsZero() && insertPos < dependencyEnd) {
        insertPos = dependencyEnd;
    }
    Position insertEnd = insertPos;
    Position cleanEnd = isMemberField ? GetFieldBlockEndLineStart(funcDecl, insertPos) : Position{};
    if (cleanEnd.IsZero()) {
        cleanEnd = {funcDecl.GetBegin().fileID, funcDecl.GetBegin().line, 1};
    }
    if (isMemberField && sel.arkAst && sel.arkAst->sourceManager && insertPos < cleanEnd) {
        std::string gap = sel.arkAst->sourceManager->GetContentBetween(insertPos, cleanEnd);
        bool whitespaceOnly = std::all_of(gap.begin(), gap.end(), [](unsigned char ch) {
            return std::isspace(ch) != 0;
        });
        if (whitespaceOnly) {
            insertEnd = cleanEnd;
        }
    }
    return Range{insertPos, insertEnd};
}

Range IntroduceField::GetAssignInsertRange(const Selection &sel, Range &range)
{
    Range insertRange;
    if (!sel.arkAst) {
        return insertRange;
    }
    insertRange.start = {range.start.fileID, range.start.line, 1};
    insertRange.end = insertRange.start;
    return insertRange;
}

static std::string GetLineIndent(const Tweak::Selection &sel, int line)
{
    if (!sel.arkAst) {
        return "";
    }
    int firstToken = GetFirstTokenOnCurLine(sel.arkAst->tokens, line);
    if (firstToken == -1) {
        return "";
    }
    int column = sel.arkAst->tokens[firstToken].Begin().column;
    return std::string(column > 0 ? column - 1 : 0, ' ');
}

static std::string GetLeadingIndentBefore(const Tweak::Selection &sel, const Position &pos)
{
    if (!sel.arkAst || !sel.arkAst->sourceManager) {
        return "";
    }
    std::string beforePos = sel.arkAst->sourceManager->GetContentBetween({pos.fileID, pos.line, 1}, pos);
    std::string indent;
    for (char ch : beforePos) {
        if (ch != ' ' && ch != '\t') {
            break;
        }
        indent.push_back(ch);
    }
    return indent;
}

static TextEdit BuildReplaceExprWithText(const Tweak::Selection &sel, Range range, const std::string &replacement)
{
    TextEdit textEdit;
    if (!sel.arkAst || !sel.arkAst->sourceManager) {
        return textEdit;
    }
    textEdit.newText = replacement;
    std::string charContent = sel.arkAst->sourceManager->GetContentBetween(
        {range.start.fileID, range.start.line, 1}, range.start);
    range.start.column = CountUnicodeCharacters(charContent) + 1;

    std::string endCharContent = sel.arkAst->sourceManager->GetContentBetween(
        {range.end.fileID, range.end.line, 1}, range.end);
    range.end.column = CountUnicodeCharacters(endCharContent) + 1;

    textEdit.range = TransformFromChar2IDE(range);

    return textEdit;
}

static std::optional<Tweak::Effect> BuildMemberInitializerEffect(const Tweak::Selection &sel, Range &range,
    const std::string &fieldName, const std::string &typeName, const MemberInitializerTarget &target)
{
    if (!sel.arkAst || !sel.arkAst->sourceManager || !sel.arkAst->file || !target.decl) {
        return std::nullopt;
    }

    TextEdit declarationEdit;
    Range insertRange{{target.decl->begin.fileID, target.decl->begin.line, 1},
        {target.decl->begin.fileID, target.decl->begin.line, 1}};
    declarationEdit.range = TransformFromChar2IDE(insertRange);
    std::string indent = target.isGlobal ? "" : GetLineIndent(sel, target.decl->begin.line);
    if (indent.empty()) {
        indent = GetLeadingIndentBefore(sel, target.decl->begin);
    }
    std::string initializer = sel.arkAst->sourceManager->GetContentBetween(range.start, range.end);
    std::ostringstream declarationText;
    declarationText << indent << "private ";
    if (!target.isGlobal && target.decl->TestAttr(Attribute::STATIC)) {
        declarationText << "static ";
    }
    declarationText << "var " << fieldName << ": " << typeName << " = " << initializer << "\n\n";
    declarationEdit.newText = declarationText.str();

    Tweak::Effect effect;
    std::vector<TextEdit> textEdits;
    textEdits.push_back(std::move(declarationEdit));
    textEdits.push_back(BuildReplaceExprWithText(sel, range, fieldName));
    std::string uri = URI::URIFromAbsolutePath(sel.arkAst->file->filePath).ToString();
    effect.applyEdits.emplace(uri, std::move(textEdits));
    effect.format = false;
    return std::move(effect);
}

static std::string GetFieldIndent(const Tweak::Selection &sel, const Range &insertRange, bool isMemberField)
{
    if (!isMemberField) {
        return "";
    }
    if (insertRange.start.column == 1) {
        std::string indent = GetLineIndent(sel, insertRange.start.line);
        if (!indent.empty()) {
            return indent;
        }
    }
    return "    ";
}

static std::string GetMemberFieldSuffix()
{
    return "\n\n";
}

static std::optional<Position> GetOwnerLeftCurlPos(Cangjie::AST::FuncDecl &funcDecl)
{
    auto owner = funcDecl.outerDecl ? DynamicCast<Cangjie::AST::InheritableDecl *>(funcDecl.outerDecl.get()) : nullptr;
    if (!owner) {
        return std::nullopt;
    }
    if (auto classDecl = DynamicCast<Cangjie::AST::ClassDecl *>(owner)) {
        if (!classDecl->body) {
            return std::nullopt;
        }
        return classDecl->body->leftCurlPos;
    } else if (auto structDecl = DynamicCast<Cangjie::AST::StructDecl *>(owner)) {
        if (!structDecl->body) {
            return std::nullopt;
        }
        return structDecl->body->leftCurlPos;
    }
    return std::nullopt;
}

static bool IsFirstLineInOwnerBody(const Position &insertPos, Cangjie::AST::FuncDecl &funcDecl)
{
    auto leftCurlPos = GetOwnerLeftCurlPos(funcDecl);
    return leftCurlPos.has_value() && insertPos.line == leftCurlPos->line + 1 && insertPos.column == 1;
}

static bool HasBlankLineAfterOwnerLeftCurl(const Tweak::Selection &sel,
                                           const Range &insertRange,
                                           Cangjie::AST::FuncDecl &funcDecl)
{
    if (!sel.arkAst || !sel.arkAst->sourceManager) {
        return false;
    }
    auto leftCurlPos = GetOwnerLeftCurlPos(funcDecl);
    if (!leftCurlPos.has_value() || *leftCurlPos >= insertRange.start) {
        return false;
    }
    std::string gap = sel.arkAst->sourceManager->GetContentBetween(*leftCurlPos, insertRange.start);
    return std::count(gap.begin(), gap.end(), '\n') >= BLANK_LINE_NEWLINE_COUNT;
}

static std::string GetMemberFieldPrefix(const Range &insertRange,
                                        const Tweak::Selection &sel,
                                        Cangjie::AST::FuncDecl &funcDecl,
                                        bool insertAtLineStart)
{
    if (!insertAtLineStart) {
        return "\n";
    }
    if (!IsFirstLineInOwnerBody(insertRange.start, funcDecl)) {
        return "";
    }
    return HasBlankLineAfterOwnerLeftCurl(sel, insertRange, funcDecl) ? "" : "\n";
}

static std::string GetFieldAccessText(Cangjie::AST::FuncDecl &funcDecl, const std::string &fieldName)
{
    return IntroduceField::IsMemberFieldTarget(funcDecl) && !IntroduceField::IsStaticFieldTarget(funcDecl) ?
        "this." + fieldName : fieldName;
}

static TextEdit BuildLocalVarDeclarationReplacement(const Tweak::Selection &sel, Cangjie::AST::VarDecl &decl,
    const std::string &fieldName, Cangjie::AST::FuncDecl &funcDecl)
{
    TextEdit textEdit;
    if (!sel.arkAst || !sel.arkAst->sourceManager || !decl.initializer) {
        return textEdit;
    }
    textEdit.range = TransformFromChar2IDE({decl.begin, decl.end});
    std::string initializer = sel.arkAst->sourceManager
                                ->GetContentBetween(decl.initializer->begin, decl.initializer->end);
    textEdit.newText = GetFieldAccessText(funcDecl, fieldName) + " = " + initializer;
    return textEdit;
}

static std::vector<TextEdit> BuildLocalVarReferenceReplacements(Cangjie::AST::FuncDecl &funcDecl,
    Cangjie::AST::VarDecl &decl, const std::string &fieldName)
{
    std::vector<TextEdit> edits;
    if (!funcDecl.funcBody || !funcDecl.funcBody->body) {
        return edits;
    }
    std::string replacement = GetFieldAccessText(funcDecl, fieldName);
    Walker(funcDecl.funcBody->body.get(), [&decl, &edits, &replacement](auto node) {
        if (!node) {
            return VisitAction::STOP_NOW;
        }
        if (node->astKind != ASTKind::REF_EXPR) {
            return VisitAction::WALK_CHILDREN;
        }
        auto refExpr = DynamicCast<Cangjie::AST::RefExpr *>(node.get());
        if (!refExpr || refExpr->GetTarget().get() != &decl) {
            return VisitAction::WALK_CHILDREN;
        }
        TextEdit textEdit;
        textEdit.range = TransformFromChar2IDE({refExpr->begin, refExpr->end});
        textEdit.newText = replacement;
        edits.push_back(textEdit);
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return edits;
}

TextEdit IntroduceField::InsertFieldDeclaration(const Selection &sel, Range &range, std::string &fieldName,
    std::string &typeName, Cangjie::AST::FuncDecl &funcDecl)
{
    TextEdit textEdit;
    if (!sel.arkAst || !sel.arkAst->sourceManager) {
        return textEdit;
    }
    auto insertRange = GetFieldInsertRange(sel, range, funcDecl);
    if (!insertRange) {
        return textEdit;
    }

    textEdit.range = TransformFromChar2IDE(*insertRange);
    std::string initializer = CanUseAsFieldInitializer(sel, range, funcDecl) ?
        sel.arkAst->sourceManager->GetContentBetween(range.start, range.end) :
        GetDefaultInitializer(sel, range, typeName);
    if (initializer.empty()) {
        initializer = sel.arkAst->sourceManager->GetContentBetween(range.start, range.end);
    }
    bool isMemberField = IsMemberFieldTarget(funcDecl);
    bool insertAtLineStart = insertRange->start.column == 1;
    std::ostringstream insertText;
    insertText << GetMemberFieldPrefix(*insertRange, sel, funcDecl, insertAtLineStart);
    insertText << GetFieldIndent(sel, *insertRange, isMemberField);
    insertText << "private ";
    if (IsStaticFieldTarget(funcDecl)) {
        insertText << "static ";
    }
    insertText << "var " << fieldName << ": " << typeName << " = " << initializer;
    insertText << GetMemberFieldSuffix();
    textEdit.newText = insertText.str();
    return textEdit;
}

static TextEdit InsertDefaultFieldDeclaration(const Tweak::Selection &sel, Range &range, std::string &fieldName,
    std::string &typeName, Cangjie::AST::FuncDecl &funcDecl)
{
    TextEdit textEdit = IntroduceField::InsertFieldDeclaration(sel, range, fieldName, typeName, funcDecl);
    std::string defaultInitializer = GetDefaultInitializer(sel, range, typeName);
    if (!sel.arkAst || !sel.arkAst->sourceManager || defaultInitializer.empty()) {
        return textEdit;
    }
    std::string selectedContent = sel.arkAst->sourceManager->GetContentBetween(range.start, range.end);
    std::string selectedInitializer = " = " + selectedContent;
    auto initializerPos = textEdit.newText.rfind(selectedInitializer);
    if (initializerPos != std::string::npos) {
        textEdit.newText.replace(initializerPos, selectedInitializer.size(), " = " + defaultInitializer);
    }
    return textEdit;
}

TextEdit IntroduceField::InsertFieldAssignment(
    const Selection &sel, Range &range, std::string &fieldName, Cangjie::AST::FuncDecl &funcDecl)
{
    TextEdit textEdit;
    if (!sel.arkAst || !sel.arkAst->sourceManager) {
        return textEdit;
    }

    Range insertRange = GetAssignInsertRange(sel, range);
    if (insertRange.start.IsZero()) {
        return textEdit;
    }
    std::string sourceCode = sel.arkAst->sourceManager->GetContentBetween(range.start, range.end);
    std::string indent = GetLineIndent(sel, range.start.line);
    if (indent.empty()) {
        indent = GetLeadingIndentBefore(sel, range.start);
    }
    textEdit.range = TransformFromChar2IDE(insertRange);
    std::ostringstream insertText;
    insertText << indent;
    if (IsMemberFieldTarget(funcDecl) && !IsStaticFieldTarget(funcDecl)) {
        insertText << "this.";
    }
    insertText << fieldName << " = " << sourceCode << "\n";
    textEdit.newText = insertText.str();
    return textEdit;
}

TextEdit IntroduceField::ReplaceExprWithField(
    const Selection &sel, Range &range, std::string &fieldName, Cangjie::AST::FuncDecl &funcDecl)
{
    std::string replacement = IsMemberFieldTarget(funcDecl) && !IsStaticFieldTarget(funcDecl) ? "this." + fieldName :
        fieldName;
    return BuildReplaceExprWithText(sel, range, replacement);
}
} // namespace ark
