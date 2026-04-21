// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "InlineVariable.h"
#include "../../../CompilerCangjieProject.h"
#include "../TweakRule.h"
#include "../TweakUtils.h"
#include "../../../common/PositionResolver.h"

// LCOV_EXCL_START
namespace ark {
    // std::vector<std::string> lowPriorityOps = {"+", "-", "==", "!=",
    //         "<", "<=", ">", ">=", "&&", "||"};
    const std::unordered_set<TokenKind> BINARY_EXPRESSION_OPERATORS = {
        TokenKind::ADD, TokenKind::SUB, TokenKind::EQUAL, TokenKind::NOTEQ, TokenKind::GT,
        TokenKind::GE, TokenKind::LT, TokenKind::LE, TokenKind::AND, TokenKind::OR,
    };

class InlineVariableSelectionRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (!root || root->selected != SelectionTree::Selection::Complete) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
            return false;
        }

        if (!root->node) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
            return false;
        }

        if (root->node->astKind != ASTKind::REF_EXPR) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineVariable::InlineVariableError::NOT_VAR_DECL_OR_REF))));
            return false;
        }

        auto refExpr = DynamicCast<RefExpr*>(root->node.get());
        if (!refExpr || refExpr->TestAttr(Attribute::LEFT_VALUE)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
            return false;
        }

        auto target = refExpr->GetTarget();
        if (!target) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineVariable::InlineVariableError::NO_TARGET))));
            return false;
        }

        if (target->astKind != ASTKind::VAR_DECL) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineVariable::InlineVariableError::NOT_VAR_DECL_OR_REF))));
            return false;
        }

        auto varDecl = DynamicCast<VarDecl*>(target);
        if (!varDecl->initializer) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineVariable::InlineVariableError::NO_INIT_EXPR))));
            return false;
        }

        if (varDecl->TestAnyAttr(Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineVariable::InlineVariableError::MEMBER_VAR))));
            return false;
        }

        return true;
    }
};

bool InlineVariable::Prepare(const Selection &sel)
{
    TweakRuleEngine ruleEngine;

    ruleEngine.AddRule(std::make_unique<InlineVariableSelectionRule>());

    return ruleEngine.CheckRules(sel, extraOptions);
}

VarDecl* InlineVariable::GetVarDecl(const Selection &sel)
{
    auto root = sel.selectionTree.root();
    if (!root || !root->node || root->node->astKind != ASTKind::REF_EXPR) {
        return nullptr;
    }

    auto refExpr = DynamicCast<RefExpr*>(root->node.get());
    if (!refExpr) {
        return nullptr;
    }

    auto target = refExpr->GetTarget();
    if (!target || target->astKind != ASTKind::VAR_DECL) {
        return nullptr;
    }

    return DynamicCast<VarDecl*>(target);
}

std::string InlineVariable::GetSourceCode(Ptr<Node> node, const Selection &sel)
{
    if (!node || !sel.arkAst || !sel.arkAst->sourceManager) {
        return "";
    }
    return sel.arkAst->sourceManager->GetContentBetween(node->begin, node->end);
}

bool InlineVariable::NeedsParentheses(Expr* initExpr, const Selection &sel)
{
    if (!initExpr) {
        return false;
    }

    if (initExpr->astKind == ASTKind::BINARY_EXPR) {
        auto binaryExpr = DynamicCast<BinaryExpr*>(initExpr);
        if (!binaryExpr) {
            return false;
        }

        if (BINARY_EXPRESSION_OPERATORS.count(binaryExpr->op)) {
            return true;
        }
    }

    return false;
}

TextEdit InlineVariable::ReplaceRefWithInitExpr(const Selection &sel, RefExpr* refExpr, const std::string &initExprCode)
{
    TextEdit edit;

    if (!refExpr) {
        return edit;
    }

    Range refRange = {refExpr->begin, refExpr->end};
    refRange = TransformFromChar2IDE(refRange);
    edit.range = refRange;
    edit.newText = initExprCode;

    return edit;
}

std::optional<Tweak::Effect> InlineVariable::Apply(const Selection &sel)
{
    Effect effect;

    if (!sel.arkAst || !sel.arkAst->sourceManager || !sel.arkAst->file) {
        return std::nullopt;
    }

    auto varDecl = GetVarDecl(sel);
    if (!varDecl || !varDecl->initializer) {
        return std::nullopt;
    }

    auto root = sel.selectionTree.root();
    if (!root || !root->node || root->node->astKind != ASTKind::REF_EXPR) {
        return std::nullopt;
    }

    auto refExpr = DynamicCast<RefExpr*>(root->node.get());
    if (!refExpr) {
        return std::nullopt;
    }

    std::string initExprCode = GetSourceCode(varDecl->initializer, sel);

    bool needsParens = NeedsParentheses(varDecl->initializer.get(), sel);
    if (needsParens) {
        initExprCode = "(" + initExprCode + ")";
    }

    std::vector<TextEdit> textEdits;

    TextEdit replaceEdit = ReplaceRefWithInitExpr(sel, refExpr, initExprCode);
    textEdits.push_back(replaceEdit);

    std::string filePath = sel.arkAst->file->filePath;
    std::string uri = URI::URIFromAbsolutePath(filePath).ToString();
    effect.applyEdits.emplace(uri, std::move(textEdits));

    return effect;
}

} // namespace ark
// LCOV_EXCL_STOP