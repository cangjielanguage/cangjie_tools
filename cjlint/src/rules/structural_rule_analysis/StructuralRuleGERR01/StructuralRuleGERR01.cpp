// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "StructuralRuleGERR01.h"
#include "common/CommonFunc.h"

namespace Cangjie::CodeCheck {
using namespace Cangjie;
using namespace AST;

void StructuralRuleGERR01::AnalyzeCatchBlock(std::vector<OwnedPtr<Cangjie::AST::Block>>& blocks)
{
    for (auto& block : blocks) {
        if (block->body.empty() || block->body[0]->TestAttr(Attribute::COMPILER_ADD)) {
            Diagnose(block->leftCurlPos, block->rightCurlPos, CodeCheckDiagKind::G_ERR_01_exceptions_process_01);
        }
    }
}

bool StructuralRuleGERR01::CommentsIncludeExceptionInfo(
    const std::vector<AST::CommentGroup>& comments, const std::string& exception)
{
    for (auto& comment : comments) {
        for (auto& c : comment.cms) {
            if (c.info.Value().find(exception) != std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

void StructuralRuleGERR01::AnalyzeThrowExpr(
    const Ptr<AST::ThrowExpr>& throwExpr, const std::vector<CommentGroup>& comments)
{
    if (throwExpr->expr->astKind == ASTKind::CALL_EXPR) {
        auto pCallExpr = As<ASTKind::CALL_EXPR>(throwExpr->expr);
        if (!pCallExpr->baseFunc) {
            return;
        }
        auto exception = pCallExpr->baseFunc->ToString();
        if (!CommentsIncludeExceptionInfo(comments, exception)) {
            Diagnose(throwExpr->begin, throwExpr->end, CodeCheckDiagKind::G_ERR_01_exceptions_process_02);
        }
    }
}

void StructuralRuleGERR01::AnalyzeFunctionDecl(Ptr<AST::Node> node, const std::vector<CommentGroup>& comments)
{
    if (!node) {
        return;
    }

    Walker walker(node, [this, &comments](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::TRY_EXPR) {
            auto pTryExpr = As<ASTKind::TRY_EXPR>(node.get());
            AnalyzeCatchBlock(pTryExpr->catchBlocks);
        }
        if (node->astKind == ASTKind::THROW_EXPR) {
            auto pThrowExpr = As<ASTKind::THROW_EXPR>(node.get());
            AnalyzeThrowExpr(pThrowExpr, comments);
        }
        if (node->astKind == ASTKind::FUNC_DECL) {
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGERR01::CheckFuncDeclHelper(
    const Ptr<AST::FuncDecl>& funcDecl, const std::vector<CommentGroup>& comments)
{
    if (!funcDecl->funcBody || !funcDecl->funcBody->body) {
        return;
    }

    for (auto& item : funcDecl->funcBody->body->body) {
        AnalyzeFunctionDecl(item.get(), comments);
    }
}

void StructuralRuleGERR01::CheckFuncDecl(Ptr<Cangjie::AST::Node>& node)
{
    if (!node) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::MAIN_DECL) {
            auto pMainDecl = As<ASTKind::MAIN_DECL>(node.get());
            if (!pMainDecl->desugarDecl) {
                return VisitAction::SKIP_CHILDREN;
            }
            auto comments = pMainDecl->desugarDecl->comments.leadingComments;
            CheckFuncDeclHelper(pMainDecl->desugarDecl.get(), comments);
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind == ASTKind::FUNC_DECL) {
            auto pFuncDecl = As<ASTKind::FUNC_DECL>(node.get());
            auto comments = pFuncDecl->comments.leadingComments;
            CheckFuncDeclHelper(pFuncDecl, comments);
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGERR01::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckFuncDecl(node);
}
} // namespace Cangjie::CodeCheck
