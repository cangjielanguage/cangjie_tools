// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "InlineFunction.h"
#include "../../../CompilerCangjieProject.h"
#include "../TweakRule.h"
#include "../TweakUtils.h"
#include "../../../common/PositionResolver.h"
#include <sstream>
#include <regex>

// LCOV_EXCL_START
namespace ark {

class InlineFunctionSelectionRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (!root || root->selected != SelectionTree::Selection::Complete) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
            return false;
        }

        if (root->node->astKind != ASTKind::CALL_EXPR) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineFunction::InlineFunctionError::NOT_CALL_EXPR))));
            return false;
        }

        auto callExpr = DynamicCast<CallExpr*>(root->node.get());
        if (!callExpr) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(TweakRule::TweakError::ERROR_AST))));
            return false;
        }

        if (!callExpr->resolvedFunction) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineFunction::InlineFunctionError::FUNC_DECL_NOT_FOUND))));
            return false;
        }

        auto funcDecl = DynamicCast<FuncDecl*>(callExpr->resolvedFunction.get());
        if (!funcDecl || !funcDecl->funcBody) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineFunction::InlineFunctionError::FUNC_DECL_NOT_FOUND))));
            return false;
        }

        return true;
    }
};

class InlineFunctionNoRecursiveRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (!root || !root->node || root->node->astKind != ASTKind::CALL_EXPR) {
            return false;
        }

        auto callExpr = DynamicCast<CallExpr*>(root->node.get());
        if (!callExpr || !callExpr->resolvedFunction) {
            return false;
        }

        auto funcDecl = DynamicCast<FuncDecl*>(callExpr->resolvedFunction.get());
        if (!funcDecl || !funcDecl->funcBody || !funcDecl->funcBody->body) {
            return false;
        }

        std::string funcName = funcDecl->identifier;
        bool isRecursive = false;

        Walker(funcDecl->funcBody->body.get(), nullptr,
            [&funcName, &isRecursive](Ptr<Node> node) -> VisitAction {
            if (node->astKind == ASTKind::CALL_EXPR) {
                auto innerCallExpr = DynamicCast<CallExpr*>(node.get());
                if (innerCallExpr && innerCallExpr->baseFunc && innerCallExpr->baseFunc->ToString() == funcName) {
                    isRecursive = true;
                    return VisitAction::STOP_NOW;
                }
            }
            return VisitAction::WALK_CHILDREN;
        }).Walk();

        if (isRecursive) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineFunction::InlineFunctionError::IS_RECURSIVE_FUNC))));
            return false;
        }

        return true;
    }
};

class InlineFunctionNoPrivateAccessRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (!root || !root->node || root->node->astKind != ASTKind::CALL_EXPR) {
            return false;
        }

        auto callExpr = DynamicCast<CallExpr*>(root->node.get());
        if (!callExpr || !callExpr->resolvedFunction) {
            return false;
        }

        auto funcDecl = DynamicCast<FuncDecl*>(callExpr->resolvedFunction.get());
        if (!funcDecl || !funcDecl->funcBody || !funcDecl->funcBody->body) {
            return false;
        }

        bool hasPrivateAccess = false;

        Walker(funcDecl->funcBody->body.get(),
            nullptr, [&hasPrivateAccess](Ptr<Node> node) -> VisitAction {
            if (node->astKind == ASTKind::REF_EXPR) {
                auto refExpr = DynamicCast<RefExpr*>(node.get());
                if (refExpr && refExpr->GetTarget() && refExpr->GetTarget()->TestAttr(Attribute::PRIVATE)) {
                    hasPrivateAccess = true;
                    return VisitAction::STOP_NOW;
                }
            }
            return VisitAction::WALK_CHILDREN;
        }).Walk();

        if (hasPrivateAccess) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineFunction::InlineFunctionError::HAS_PRIVATE_ACCESS))));
            return false;
        }

        return true;
    }
};

class InlineFunctionSimpleTypeRule : public TweakRule {
    bool Check(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions) const override
    {
        auto root = sel.selectionTree.root();
        if (!root || !root->node || root->node->astKind != ASTKind::CALL_EXPR) {
            return false;
        }

        auto callExpr = DynamicCast<CallExpr*>(root->node.get());
        if (!callExpr || !callExpr->resolvedFunction) {
            return false;
        }

        auto funcDecl = DynamicCast<FuncDecl*>(callExpr->resolvedFunction.get());
        if (!funcDecl || !funcDecl->funcBody) {
            return false;
        }

        if (funcDecl->funcBody->generic) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineFunction::InlineFunctionError::IS_GENERIC_FUNC))));
            return false;
        }

        if (funcDecl->TestAnyAttr(Attribute::IN_CLASSLIKE, Attribute::IN_STRUCT, Attribute::IN_ENUM)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineFunction::InlineFunctionError::IS_MEMBER_FUNC))));
            return false;
        }

        if (funcDecl->TestAttr(Attribute::IN_EXTEND)) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineFunction::InlineFunctionError::IS_EXTEND_FUNC))));
            return false;
        }

        if (callExpr->baseFunc && callExpr->baseFunc->astKind == ASTKind::LAMBDA_EXPR) {
            extraOptions.insert(std::make_pair("ErrorCode",
                std::to_string(static_cast<int>(InlineFunction::InlineFunctionError::IS_LAMBDA_CALL))));
            return false;
        }

        return true;
    }
};

bool InlineFunction::Prepare(const Selection &sel)
{
    TweakRuleEngine ruleEngine;

    ruleEngine.AddRule(std::make_unique<InlineFunctionSelectionRule>());
    ruleEngine.AddRule(std::make_unique<InlineFunctionNoRecursiveRule>());
    ruleEngine.AddRule(std::make_unique<InlineFunctionNoPrivateAccessRule>());
    ruleEngine.AddRule(std::make_unique<InlineFunctionSimpleTypeRule>());

    return ruleEngine.CheckRules(sel, extraOptions);
}

CallExpr* InlineFunction::GetCallExpr()
{
    auto root = sel_->selectionTree.root();
    if (!root || !root->node || root->node->astKind != ASTKind::CALL_EXPR) {
        return nullptr;
    }
    return DynamicCast<CallExpr*>(root->node.get());
}

FuncDecl* InlineFunction::GetFuncDecl()
{
    if (!callExpr_ || !callExpr_->resolvedFunction) {
        return nullptr;
    }
    return DynamicCast<FuncDecl*>(callExpr_->resolvedFunction.get());
}

bool InlineFunction::HasReturnValue()
{
    if (!funcDecl_ || !funcDecl_->funcBody) {
        return false;
    }

    if (funcDecl_->funcBody->retType) {
        auto retTy = funcDecl_->funcBody->retType.get();
        if (retTy && retTy->ty) {
            auto typeStr = retTy->ty->name;
            return typeStr != "void" && typeStr != "Unit";
        }
    }

    return false;
}

std::string InlineFunction::GetSourceCode(Ptr<Node> node)
{
    if (!node || !sel_->arkAst || !sel_->arkAst->sourceManager) {
        return "";
    }
    return sel_->arkAst->sourceManager->GetContentBetween(node->begin, node->end);
}

std::string InlineFunction::GetReturnTypeString()
{
    if (!funcDecl_ || !funcDecl_->funcBody || !funcDecl_->funcBody->retType) {
        return "";
    }

    auto retTy = funcDecl_->funcBody->retType.get();
    if (!retTy || !retTy->ty) {
        return "";
    }

    return retTy->ty->name;
}

std::string InlineFunction::ReplaceParamsInCode(const std::string &code,
    const std::map<std::string, std::string> &paramMap)
{
    std::string result = code;
    for (const auto &[paramName, paramValue] : paramMap) {
        std::regex paramRegex("\\b" + paramName + "\\b");
        result = std::regex_replace(result, paramRegex, paramValue);
    }
    return result;
}

std::string InlineFunction::GenerateInlineVarName()
{
    return "inline_result";
}

std::string InlineFunction::GenerateParamVarName(const std::string &paramName)
{
    return "inline_arg_" + paramName;
}

bool InlineFunction::IsSimpleArg(Expr* expr)
{
    if (!expr) {
        return true;
    }

    if (expr->astKind == ASTKind::REF_EXPR) {
        return true;
    }

    if (expr->astKind == ASTKind::LIT_CONST_EXPR) {
        return true;
    }

    return false;
}

bool InlineFunction::IsComplexArg(Expr* expr)
{
    return !IsSimpleArg(expr);
}

std::string InlineFunction::GetParamTypeString(FuncParam* param)
{
    if (!param || !param->ty) {
        return "";
    }
    return param->ty->name;
}

void InlineFunction::ExtractParams()
{
    if (!callExpr_ || !funcDecl_ || !funcDecl_->funcBody) {
        return;
    }

    size_t callArgCount = callExpr_->args.size();
    size_t defaultArgCount = callExpr_->defaultArgs.size();

    size_t argIndex = 0;
    for (const auto &param : funcDecl_->funcBody->paramLists[0]->params) {
        std::string paramName = param->identifier;
        std::string paramType = GetParamTypeString(param.get());
        std::string paramValue;
        bool needsExtract = false;

        if (argIndex < callArgCount) {
            auto &arg = callExpr_->args[argIndex];
            if (arg && arg->expr) {
                paramValue = GetSourceCode(arg->expr);
                needsExtract = IsComplexArg(arg->expr.get());
            }
        } else if (argIndex < callArgCount + defaultArgCount) {
            size_t defaultArgIndex = argIndex - callArgCount;
            auto &defaultArg = callExpr_->defaultArgs[defaultArgIndex];
            if (defaultArg && defaultArg->expr) {
                paramValue = GetSourceCode(defaultArg->expr);
                needsExtract = true;
            }
        }

        extractedParams_.params.emplace_back(paramName, paramType, paramValue, needsExtract);
        argIndex++;
    }
}

TextEdit InlineFunction::InsertParamDecls()
{
    TextEdit edit;

    std::ostringstream paramDecls;
    bool hasParamDecls = false;

    for (const auto &[paramName, paramType, paramValue, needsExtract] : extractedParams_.params) {
        if (needsExtract && !paramValue.empty()) {
            std::string varName = GenerateParamVarName(paramName);
            paramDecls << "var " << varName;
            if (!paramType.empty()) {
                paramDecls << ": " << paramType;
            }
            paramDecls << " = " << paramValue << "\n";
            hasParamDecls = true;
        }
    }

    if (!hasParamDecls) {
        return edit;
    }

    std::string fullIndent;
    Range insertRange = FindInsertPosition(fullIndent);
    if (insertRange.end.IsZero()) {
        return edit;
    }

    std::ostringstream insertText;
    std::istringstream declStream(paramDecls.str());
    std::string line;
    while (std::getline(declStream, line)) {
        if (!line.empty()) {
            insertText << fullIndent << line << "\n";
        }
    }
    insertText << fullIndent;

    insertRange = TransformFromChar2IDE(insertRange);
    edit.range = insertRange;
    edit.newText = insertText.str();

    return edit;
}

Range InlineFunction::FindInsertPosition(std::string &indent)
{
    Range insertRange;
    if (!callExpr_ || !sel_->arkAst || sel_->arkAst->tokens.empty()) {
        return insertRange;
    }

    bool isGlobal = false;
    Range callRange = {callExpr_->begin, callExpr_->end};

    insertRange = FindInsertPositionFromBlock(callRange, isGlobal, indent);
    if (!insertRange.end.IsZero()) {
        return insertRange;
    }

    if (isGlobal && sel_->arkAst->file) {
        insertRange = TweakUtils::FindGlobalInsertPos(*sel_->arkAst->file, callRange);
        indent = "";
        return insertRange;
    }

    return FindInsertPositionFromToken(indent);
}

Range InlineFunction::FindInsertPositionFromBlock(Range &callRange, bool &isGlobal, std::string &indent)
{
    Range insertRange;
    auto outerExpr = sel_->selectionTree.OuterInterpExpr();
    if (!outerExpr) {
        return insertRange;
    }

    auto expr = DynamicCast<Expr*>(outerExpr);
    if (!expr) {
        return insertRange;
    }

    auto ctx = sel_->arkAst->packageInstance ? sel_->arkAst->packageInstance->ctx : nullptr;
    if (!ctx) {
        return insertRange;
    }

    auto block = TweakUtils::FindOuterScopeNode(*ctx, *expr, isGlobal, callRange);
    if (!block) {
        return insertRange;
    }

    for (const auto &subNode : block->body) {
        if (!subNode || subNode->begin > callRange.start || subNode->end < callRange.end) {
            continue;
        }
        insertRange = {subNode->begin, subNode->begin};
        indent = GetIndent(subNode->begin);
        return insertRange;
    }

    return insertRange;
}

Range InlineFunction::FindInsertPositionFromToken(std::string &indent)
{
    Range insertRange;
    int firstTokenIdx = GetFirstTokenOnCurLine(sel_->arkAst->tokens, callExpr_->begin.line);
    if (firstTokenIdx < 0 || firstTokenIdx >= static_cast<int>(sel_->arkAst->tokens.size())) {
        return insertRange;
    }

    Token firstToken = sel_->arkAst->tokens[firstTokenIdx];
    insertRange = {firstToken.Begin(), firstToken.Begin()};
    indent = GetIndent(firstToken.Begin());
    return insertRange;
}

TextEdit InlineFunction::InsertInlineBody()
{
    TextEdit edit;

    if (!callExpr_ || !funcDecl_ || !funcDecl_->funcBody || !funcDecl_->funcBody->body) {
        return edit;
    }

    std::string indent;
    Range insertRange = FindInsertPosition(indent);
    if (insertRange.end.IsZero()) {
        return edit;
    }

    std::map<std::string, std::string> paramMap;
    for (const auto &[paramName, paramType, paramValue, needsExtract] : extractedParams_.params) {
        if (needsExtract) {
            paramMap[paramName] = GenerateParamVarName(paramName);
        } else {
            paramMap[paramName] = paramValue;
        }
    }

    std::string returnType = GetReturnTypeString();

    std::string inlinedBody = TransformFunctionBody(
        funcDecl_->funcBody->body.get(), paramMap);

    std::ostringstream insertText;

    if (hasReturnValue_) {
        insertText << "var " << resultVarName_;
        if (!returnType.empty()) {
            insertText << ": " << returnType;
        }
        insertText << "\n";
    }

    if (!inlinedBody.empty()) {
        std::istringstream bodyStream(inlinedBody);
        std::string line;
        while (std::getline(bodyStream, line)) {
            if (!line.empty()) {
                insertText << indent << line << "\n";
            }
        }
    }

    insertText << indent;

    insertRange = TransformFromChar2IDE(insertRange);
    edit.range = insertRange;
    edit.newText = insertText.str();

    return edit;
}

TextEdit InlineFunction::ReplaceCallWithVar()
{
    TextEdit edit;

    if (!callExpr_) {
        return edit;
    }

    Range callRange = {callExpr_->begin, callExpr_->end};
    callRange = TransformFromChar2IDE(callRange);
    edit.range = callRange;
    edit.newText = resultVarName_;

    return edit;
}

std::string InlineFunction::GetIndent(Position pos)
{
    if (!sel_->arkAst || sel_->arkAst->tokens.empty()) {
        return "";
    }

    int firstTokenIdx = GetFirstTokenOnCurLine(sel_->arkAst->tokens, pos.line);
    if (firstTokenIdx < 0 || firstTokenIdx >= static_cast<int>(sel_->arkAst->tokens.size())) {
        return "";
    }

    Token firstToken = sel_->arkAst->tokens[firstTokenIdx];
    int column = firstToken.Begin().column;
    return std::string(column > 0 ? column - 1 : 0, ' ');
}

std::string InlineFunction::TransformFunctionBody(Block* block, const std::map<std::string, std::string> &paramMap)
{
    if (!block) {
        return "";
    }

    std::ostringstream bodyCode;

    for (size_t i = 0; i < block->body.size(); ++i) {
        auto stmt = block->body[i].get();
        if (!stmt) {
            continue;
        }

        if (stmt->astKind == ASTKind::RETURN_EXPR) {
            auto returnExpr = DynamicCast<ReturnExpr*>(stmt);
            std::string returnCode = TransformReturnStatement(returnExpr, paramMap);
            if (!returnCode.empty()) {
                bodyCode << returnCode;
            }
        } else {
            AppendStatementCode(bodyCode, stmt, paramMap);
        }
    }

    return bodyCode.str();
}

std::string InlineFunction::TransformReturnStatement(ReturnExpr* returnExpr,
    const std::map<std::string, std::string> &paramMap)
{
    if (!returnExpr || !returnExpr->expr || !hasReturnValue_ || resultVarName_.empty()) {
        return "";
    }

    std::string returnValue = GetSourceCode(returnExpr->expr);
    returnValue = ReplaceParamsInCode(returnValue, paramMap);
    return resultVarName_ + " = " + returnValue + "\n";
}

void InlineFunction::AppendStatementCode(std::ostringstream &bodyCode, Node* stmt,
    const std::map<std::string, std::string> &paramMap)
{
    std::string stmtCode = GetSourceCode(stmt);
    stmtCode = ReplaceParamsInCode(stmtCode, paramMap);
    bodyCode << stmtCode;
    if (stmtCode.back() != '\n') {
        bodyCode << "\n";
    }
}

std::optional<Tweak::Effect> InlineFunction::Apply(const Selection &sel)
{
    Effect effect;

    if (!sel.arkAst || !sel.arkAst->sourceManager || !sel.arkAst->file) {
        return std::nullopt;
    }

    sel_ = &sel;
    callExpr_ = GetCallExpr();
    if (!callExpr_) {
        return std::nullopt;
    }

    funcDecl_ = GetFuncDecl();
    if (!funcDecl_ || !funcDecl_->funcBody || !funcDecl_->funcBody->body) {
        return std::nullopt;
    }

    hasReturnValue_ = HasReturnValue();
    resultVarName_ = GenerateInlineVarName();

    ExtractParams();

    std::vector<TextEdit> textEdits;

    indent_ = GetIndent(callExpr_->begin);
    TextEdit paramDeclsEdit = InsertParamDecls();
    if (!paramDeclsEdit.newText.empty()) {
        textEdits.push_back(paramDeclsEdit);
    }

    TextEdit insertBodyEdit = InsertInlineBody();
    if (!insertBodyEdit.newText.empty()) {
        textEdits.push_back(insertBodyEdit);
    }

    if (hasReturnValue_) {
        TextEdit replaceCallEdit = ReplaceCallWithVar();
        textEdits.push_back(replaceCallEdit);
    }

    std::string filePath = sel.arkAst->file->filePath;
    std::string uri = URI::URIFromAbsolutePath(filePath).ToString();
    effect.applyEdits.emplace(uri, std::move(textEdits));

    return effect;
}

} // namespace ark