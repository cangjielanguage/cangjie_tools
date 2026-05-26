// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "TweakUtils.h"
#include <algorithm>
#include <cctype>
#include <cangjie/AST/Walker.h>
// LCOV_EXCL_START
namespace ark {
namespace {
constexpr size_t INDENT_RESERVE_EXTRA_LINES = 2;
}

bool TweakUtils::Contain(Node &node, Range &range)
{
    return node.begin <= range.start && node.end >= range.end;
}

size_t TweakUtils::FindTokenBoundaryPos(const std::string &text, const std::string &token)
{
    size_t searchPos = 0;
    while (searchPos < text.size()) {
        size_t pos = text.find(token, searchPos);
        if (pos == std::string::npos) {
            return std::string::npos;
        }
        bool leftBoundary = (pos == 0) || (!std::isalnum(static_cast<unsigned char>(text[pos - 1])) &&
            text[pos - 1] != '_');
        size_t endPos = pos + token.size();
        bool rightBoundary = endPos >= text.size() || (!std::isalnum(static_cast<unsigned char>(text[endPos])) &&
            text[endPos] != '_');
        if (leftBoundary && rightBoundary) {
            return pos;
        }
        searchPos = endPos;
    }
    return std::string::npos;
}

void TweakUtils::AdvancePosition(Cangjie::Position &pos, const std::string &text, size_t from, size_t to)
{
    size_t offset = from;
    while (offset < to) {
        size_t newline = text.find('\n', offset);
        if (newline == std::string::npos || newline >= to) {
            pos.column += static_cast<int>(CountUnicodeCharacters(text.substr(offset, to - offset)));
            break;
        }
        pos.column += static_cast<int>(CountUnicodeCharacters(text.substr(offset, newline - offset)));
        pos.line += 1;
        pos.column = 1;
        offset = newline + 1;
    }
}

Cangjie::Position TweakUtils::PositionAtOffset(Cangjie::Position start, const std::string &text, size_t offset)
{
    AdvancePosition(start, text, 0, offset);
    return start;
}

std::string TweakUtils::NormalizeSignature(const std::string &signature)
{
    std::string normalized;
    normalized.reserve(signature.size());
    for (char ch : signature) {
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            continue;
        }
        normalized.push_back(ch);
    }
    return normalized;
}

std::string TweakUtils::IndentTextBlock(const std::string &text, const std::string &indent)
{
    if (text.empty()) {
        return "";
    }
    std::string normalized = text;
    normalized.erase(std::remove(normalized.begin(), normalized.end(), '\r'), normalized.end());
    std::string indented;
    indented.reserve(normalized.size() + indent.size() * INDENT_RESERVE_EXTRA_LINES);
    size_t start = 0;
    while (start <= normalized.size()) {
        size_t end = normalized.find('\n', start);
        std::string line = end == std::string::npos
            ? normalized.substr(start)
            : normalized.substr(start, end - start);
        indented += indent + line;
        if (end == std::string::npos) {
            break;
        }
        indented.push_back('\n');
        start = end + 1;
    }
    return indented;
}

Ptr<Block> TweakUtils::FindOuterScopeNode(const ASTContext &ctx, const Expr &expr, bool &isGlobal, Range &range)
{
    std::string scopeName = expr.scopeName;
    if (expr.scopeName == TOPLEVEL_SCOPE_NAME) {
        isGlobal = true;
        return nullptr;
    }
    return GetSatisfiedBlock(ctx, range, scopeName, isGlobal);
}

Ptr<Block> TweakUtils::GetSatisfiedBlock(const ASTContext &ctx, Range &range, const std::string &scopeName,
    bool &isGlobal)
{
    std::string scopeGateName = ScopeManagerApi::GetScopeGateName(scopeName);
    while (!scopeGateName.empty() && scopeGateName != TOPLEVEL_SCOPE_NAME) {
        auto sym = ScopeManagerApi::GetScopeGate(ctx, scopeGateName);
        if (!sym) {
            std::string tmpScopeName = ScopeManagerApi::GetParentScopeName(scopeGateName);
            scopeGateName = ScopeManagerApi::GetScopeGateName(tmpScopeName);
            continue;
        }
        auto block = GetSymbolBlock(*sym, range);
        if (block) {
            return block;
        }
        scopeGateName = ScopeManagerApi::GetScopeGateName(sym->scopeName);
    }
    if (scopeGateName == TOPLEVEL_SCOPE_NAME) {
        isGlobal = true;
    }
    return nullptr;
}

Ptr<Block> TweakUtils::GetSatisfiedBlockBySearch(const ASTContext &ctx, Range &range, const std::string &scopeName,
    bool &isGlobal)
{
        std::string scopeGateName = scopeName;
        while (!scopeGateName.empty() && scopeGateName != TOPLEVEL_SCOPE_NAME) {
            std::string query = "scopeName = '" + scopeGateName + "'";

            auto syms = SearchContext(&ctx, query);
            if (syms.empty() || !syms[0]) {
                scopeGateName = ScopeManagerApi::GetParentScopeName(scopeGateName);
                continue;
            }
            auto sym = syms[0];
            auto block = GetSymbolBlock(*sym, range);
            if (block) {
                return block;
            }
            scopeGateName = sym->scopeName;
        }
        if (scopeGateName == TOPLEVEL_SCOPE_NAME) {
            isGlobal = true;
        }
        return nullptr;
}

Ptr<Block> TweakUtils::GetSymbolBlock(AST::Symbol &symbol, Range &range)
{
    if (!symbol.node) {
        return nullptr;
    }
    switch (symbol.node->astKind) {
        case Cangjie::AST::ASTKind::BLOCK: {
            auto block = DynamicCast<Block>(symbol.node);
            return DealBlock(block, range);
        }
        case Cangjie::AST::ASTKind::FUNC_BODY: {
            auto funcBody = DynamicCast<FuncBody>(symbol.node);
            return DealFuncBody(funcBody, range);
        }
        case Cangjie::AST::ASTKind::IF_EXPR: {
            auto ifExpr = DynamicCast<IfExpr>(symbol.node);
            return DealIfExpr(ifExpr, range);
        }
        case Cangjie::AST::ASTKind::WHILE_EXPR: {
            auto whileExpr = DynamicCast<WhileExpr>(symbol.node);
            return DealWhileExpr(whileExpr, range);
        }
        case Cangjie::AST::ASTKind::DO_WHILE_EXPR: {
            auto doWhileExpr = DynamicCast<DoWhileExpr>(symbol.node);
            return DealDoWhileExpr(doWhileExpr, range);
        }
        case Cangjie::AST::ASTKind::TRY_EXPR: {
            auto tryExpr = DynamicCast<TryExpr>(symbol.node);
            return DealTryExpr(tryExpr, range);
        }
        case Cangjie::AST::ASTKind::FOR_IN_EXPR: {
            auto forInExpr = DynamicCast<ForInExpr>(symbol.node);
            return DealForInExpr(forInExpr, range);
        }
        case Cangjie::AST::ASTKind::MATCH_CASE: {
            auto matchCase = DynamicCast<MatchCase>(symbol.node);
            return DealMatchCase(matchCase, range);
        }
        case Cangjie::AST::ASTKind::FUNC_DECL: {
            auto funcDecl = DynamicCast<FuncDecl>(symbol.node);
            return DealFuncDecl(funcDecl, range);
        }
        default:
            return nullptr;
    }
}

Ptr<Block> TweakUtils::DealTryExpr(TryExpr *tryExpr, Range &range)
{
    if (!tryExpr) {
        return nullptr;
    }
    if (tryExpr->tryBlock && Contain(*tryExpr->tryBlock, range)) {
        return tryExpr->tryBlock;
    }
    if (tryExpr->finallyBlock && Contain(*tryExpr->finallyBlock, range)) {
        return tryExpr->finallyBlock;
    }
    for (const auto &catchBlock : tryExpr->catchBlocks) {
        if (catchBlock && Contain(*catchBlock, range)) {
            return catchBlock;
        }
    }
    for (const auto &handler : tryExpr->handlers) {
        if (handler.block && Contain(*handler.block, range)) {
            return handler.block;
        }
    }
    return nullptr;
}

Ptr<Block> TweakUtils::DealBlock(Block *block, Range &range)
{
    if (!block) {
        return nullptr;
    }
    return Contain(*block, range) ? block : nullptr;
}

Ptr<Block> TweakUtils::DealFuncBody(FuncBody *funcBody, Range &range)
{
    if (!funcBody) {
        return nullptr;
    }
    if (funcBody->body && Contain(*funcBody->body, range)) {
        return funcBody->body;
    }
    return nullptr;
}

Ptr<Block> TweakUtils::DealIfExpr(IfExpr *ifExpr, Range &range)
{
    if (!ifExpr) {
        return nullptr;
    }
    if (ifExpr->thenBody && Contain(*ifExpr->thenBody, range)) {
        return ifExpr->thenBody;
    }
    return nullptr;
}

Ptr<Block> TweakUtils::DealWhileExpr(WhileExpr *whileExpr, Range &range)
{
    if (!whileExpr) {
        return nullptr;
    }
    if (whileExpr->body && Contain(*whileExpr->body, range)) {
        return whileExpr->body;
    }
    return nullptr;
}

Ptr<Block> TweakUtils::DealDoWhileExpr(DoWhileExpr *doWhileExpr, Range &range)
{
    if (!doWhileExpr) {
        return nullptr;
    }
    if (doWhileExpr->body && Contain(*doWhileExpr->body, range)) {
        return doWhileExpr->body;
    }
    return nullptr;
}

Ptr<Block> TweakUtils::DealForInExpr(ForInExpr *forInExpr, Range &range)
{
    if (!forInExpr) {
        return nullptr;
    }
    if (forInExpr->body && Contain(*forInExpr->body, range)) {
        return forInExpr->body;
    }
    return nullptr;
}

Ptr<Block> TweakUtils::DealMatchCase(MatchCase *matchCase, Range &range)
{
    if (!matchCase) {
        return nullptr;
    }
    if (matchCase->exprOrDecls && Contain(*matchCase->exprOrDecls, range)) {
        return matchCase->exprOrDecls;
    }
    return nullptr;
}

Ptr<Block> TweakUtils::DealFuncDecl(FuncDecl *funcDecl, Range &range)
{
    if (!funcDecl) {
        return nullptr;
    }
    return funcDecl->funcBody ? DealFuncBody(funcDecl->funcBody.get(), range) : nullptr;
}

bool TweakUtils::CheckValidExpr(const SelectionTree::SelectionTreeNode &treeNode)
{
    if (!treeNode.node) {
        return false;
    }
    if (treeNode.node->IsExpr()) {
        return true;
    }
    bool isValid = false;
    SelectionTree::Walk(&treeNode, [&isValid, &treeNode]
        (const SelectionTree::SelectionTreeNode *pNode) {
            if (!pNode || !pNode->node) {
                return SelectionTree::WalkAction::STOP_NOW;
            }
            if (pNode->node->IsExpr() && pNode->node->begin == treeNode.node->begin
                && pNode->node->end == treeNode.node->end) {
                isValid = true;
                return SelectionTree::WalkAction::STOP_NOW;
            }
            return SelectionTree::WalkAction::WALK_CHILDREN;
        });
    return isValid;
}

Range TweakUtils::GetCompleteExprRange(const SelectionTree &selectionTree)
{
    Range range;
    auto root = selectionTree.root();
    if (!root || !root->node || root->selected != SelectionTree::Selection::Complete) {
        return range;
    }
    range.start = root->node->begin;
    range.end = root->node->end;
    return range;
}

Ptr<FuncDecl> TweakUtils::FindEnclosingFunc(const ArkAST &arkAst, const Range &range)
{
    if (!arkAst.file || range.start == Position{0, 0, 0} || range.start == range.end) {
        return nullptr;
    }

    Ptr<FuncDecl> result = nullptr;
    for (auto &decl : arkAst.file->decls) {
        if (!decl || decl->begin > range.start || decl->end < range.end) {
            continue;
        }
        Walker(decl.get(), [&range, &result](auto node) {
            if (!node || node->begin > range.start || node->end < range.end) {
                return VisitAction::SKIP_CHILDREN;
            }
            auto funcDecl = DynamicCast<FuncDecl *>(node.get());
            if (funcDecl && funcDecl->funcBody && funcDecl->funcBody->body
                && funcDecl->funcBody->body->begin <= range.start && funcDecl->funcBody->body->end >= range.end) {
                result = funcDecl;
            }
            return VisitAction::WALK_CHILDREN;
        }).Walk();
        if (result) {
            break;
        }
    }
    return result;
}

Ptr<FuncDecl> TweakUtils::GetTargetFunc(const SelectionTree &selectionTree, const ArkAST *arkAst, const Range &range)
{
    auto targetDecl = selectionTree.TargetDecl();
    auto funcDecl = DynamicCast<FuncDecl *>(targetDecl.get());
    if (!funcDecl && arkAst) {
        funcDecl = FindEnclosingFunc(*arkAst, range);
    }
    if (!funcDecl || !funcDecl->funcBody || !funcDecl->funcBody->body) {
        return nullptr;
    }
    return funcDecl;
}

std::string TweakUtils::GetSelectedExprTypeName(const SelectionTree &selectionTree, const Range &range)
{
    std::string typeName;
    auto root = selectionTree.root();
    if (!root || !root->node) {
        return typeName;
    }
    SelectionTree::Walk(root, [&range, &typeName](const SelectionTree::SelectionTreeNode *treeNode) {
        if (!treeNode || !treeNode->node) {
            return SelectionTree::WalkAction::STOP_NOW;
        }
        if (treeNode->selected == SelectionTree::Selection::Complete && treeNode->node->IsExpr() &&
            treeNode->node->begin == range.start && treeNode->node->end == range.end) {
            auto expr = DynamicCast<Expr *>(treeNode->node.get());
            if (expr && expr->GetTy() && GetString(*expr->GetTy()) != "UnknownType") {
                typeName = GetString(*expr->GetTy());
            }
            return SelectionTree::WalkAction::STOP_NOW;
        }
        return SelectionTree::WalkAction::WALK_CHILDREN;
    });
    return typeName;
}

Range TweakUtils::FindGlobalInsertPos(const File &file, Range &range)
{
    Range insertRange;
    for (size_t i = 0; i < file.decls.size(); ++i) {
        auto decl = file.decls[i].get();
        if (!decl) {
            continue;
        }
        if (i == 0 && TweakUtils::Contain(*decl, range)) {
            Position pos = FindLastImportPos(file);
            insertRange = {pos, pos};
            break;
        }
        if (i > 0 && TweakUtils::Contain(*decl, range) && file.decls[i - 1].get()) {
            auto prevDecl = file.decls[i - 1].get();
            insertRange = {prevDecl->end, prevDecl->end};
            break;
        }
    }
    if (insertRange.end.IsZero()) {
        Position pos = FindLastImportPos(file);
        insertRange = {pos, pos};
    }
    return insertRange;
}
} // namespace ark
// LCOV_EXCL_STOP
