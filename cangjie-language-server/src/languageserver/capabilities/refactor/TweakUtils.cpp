// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "TweakUtils.h"

namespace ark {
bool TweakUtils::Contain(Node &node, Range &range)
{
    return node.begin <= range.start && node.end >= range.end;
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

Position TweakUtils::FindLastImportPos(const File &file)
{
    int lastImportLine = 0;
    for (const auto &import : file.imports) {
        if (!import) {
            continue;
        }
        lastImportLine = std::max(import->content.rightCurlPos.line, std::max(import->importPos.line, lastImportLine));
    }
    Position pkgPos = file.package->packagePos;
    if (lastImportLine == 0 && pkgPos.line > 0) {
        lastImportLine = pkgPos.line;
    }
    return {pkgPos.fileID, lastImportLine + 1, 1};
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
            Position pos = TweakUtils::FindLastImportPos(file);
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
        Position pos = TweakUtils::FindLastImportPos(file);
        insertRange = {pos, pos};
    }
    return insertRange;
}
} // namespace ark
