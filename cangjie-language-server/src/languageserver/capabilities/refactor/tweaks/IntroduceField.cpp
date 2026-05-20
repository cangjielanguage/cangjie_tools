// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "IntroduceField.h"
#include "../../../common/Utils.h"
#include "../TweakRule.h"
#include "../TweakUtils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <cangjie/AST/Walker.h>

namespace ark {
const std::unordered_set<Cangjie::AST::ASTKind> CANNOT_INTRODUCE_FIELD_EXPR = {
    ASTKind::BLOCK, ASTKind::STR_INTERPOLATION_EXPR, ASTKind::INTERPOLATION_EXPR
};

static bool CanUseAsFieldInitializer(const Tweak::Selection &sel, Cangjie::AST::FuncDecl &funcDecl)
{
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return false;
    }
    (void)funcDecl;
    return root->node->astKind == ASTKind::LIT_CONST_EXPR;
}

static std::string GetDefaultInitializer(const std::string &typeName)
{
    static const std::unordered_map<std::string, std::string> DEFAULT_INITIALIZERS = {
        {"Float64", "0.0"},
        {"Bool", "false"},
        {"String", "\"\""},
        {"Rune", "r'\\0'"},
        {"Unit", "()"},
        {"Int64", "0"},
    };

    auto it = DEFAULT_INITIALIZERS.find(typeName);
    return it != DEFAULT_INITIALIZERS.end() ? it->second : "";
}

class IntroduceFieldRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (!root || !root->node) {
            return false;
        }
        if (root->selected != SelectionTree::Selection::Complete || !TweakUtils::CheckValidExpr(*root)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::FAIL_GET_ROOT_EXPR))));
            return false;
        }
        if (CANNOT_INTRODUCE_FIELD_EXPR.count(root->node->astKind)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::INVALID_CODE_SEGMENT))));
            return false;
        }
        if (!root->node->IsExpr()) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::INVALID_EXPR))));
            return false;
        }
        auto funcDecl = IntroduceField::GetTargetFunc(sel);
        if (!funcDecl || !IntroduceField::IsSupportedTargetFunc(*funcDecl)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::INVALID_SCOPE))));
            return false;
        }
        auto range = TweakUtils::GetCompleteExprRange(sel.selectionTree);
        if (TweakUtils::GetSelectedExprTypeName(sel.selectionTree, range).empty()) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(IntroduceField::IntroduceFieldError::INVALID_TYPE))));
            return false;
        }
        return true;
    }
};

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
    auto funcDecl = GetTargetFunc(sel);
    if (!funcDecl || !IsSupportedTargetFunc(*funcDecl)) {
        return std::nullopt;
    }

    std::string fieldName = "newField";
    auto it = sel.extraOptions.find("suggestName");
    if (it != sel.extraOptions.end()) {
        fieldName = it->second;
    }

    Range range = TweakUtils::GetCompleteExprRange(sel.selectionTree);
    if (range.start == Position{0, 0, 0} || range.start == range.end) {
        return std::nullopt;
    }
    std::string typeName = TweakUtils::GetSelectedExprTypeName(sel.selectionTree, range);
    if (typeName.empty()) {
        return std::nullopt;
    }
    bool useSelectedExprAsInitializer = CanUseAsFieldInitializer(sel, *funcDecl) ||
        GetDefaultInitializer(typeName).empty();

    std::vector<TextEdit> textEdits;
    textEdits.push_back(InsertFieldDeclaration(sel, range, fieldName, typeName, *funcDecl));
    if (!useSelectedExprAsInitializer) {
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
    return funcDecl.outerDecl->astKind == ASTKind::CLASS_DECL || funcDecl.outerDecl->astKind == ASTKind::STRUCT_DECL;
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

static Position GetMethodInsertStart(
    const Tweak::Selection &sel, const Position &searchStart, Cangjie::AST::FuncDecl &funcDecl)
{
    Position codeBegin = GetCodeBegin(sel, funcDecl);
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
    Position previousDeclEnd = sel.arkAst->file->package ? sel.arkAst->file->package->GetEnd() :
        Position{funcDecl.begin.fileID, 1, 1};
    for (auto &decl : sel.arkAst->file->decls) {
        if (!decl || decl->begin >= funcDecl.begin) {
            break;
        }
        previousDeclEnd = decl->end;
    }
    return GetMethodInsertStart(sel, previousDeclEnd, funcDecl);
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
    return std::count(gap.begin(), gap.end(), '\n') >= 2;
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
    std::string initializer = CanUseAsFieldInitializer(sel, funcDecl) ?
        sel.arkAst->sourceManager->GetContentBetween(range.start, range.end) : GetDefaultInitializer(typeName);
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
    TextEdit textEdit;
    textEdit.newText = IsMemberFieldTarget(funcDecl) && !IsStaticFieldTarget(funcDecl) ? "this." + fieldName :
        fieldName;
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
} // namespace ark
