// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_INLINEFUNCTION_H
#define CANGJIE_LSP_INLINEFUNCTION_H

#include "../Tweak.h"

namespace ark {

class InlineFunction : public Tweak {
public:
    enum class InlineFunctionError {
        NOT_CALL_EXPR = 2,
        FUNC_DECL_NOT_FOUND,
        MULTIPLE_RETURN_PATHS,
        COMPLEX_FUNC_BODY,
        HAS_SIDE_EFFECT_ARGS,
        IS_GENERIC_FUNC,
        IS_MEMBER_FUNC,
        IS_EXTEND_FUNC,
        IS_LAMBDA_CALL,
        IS_RECURSIVE_FUNC,
        HAS_PRIVATE_ACCESS,
        RETURNS_CLOSURE,
        HAS_NAMED_ARGS,
        HAS_DEFAULT_ARGS
    };

    const std::string Id() const override
    {
        return "InlineFunction";
    }

    std::string Title() const override
    {
        return "Inline function";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Selection &sel) override;

    std::optional<Effect> Apply(const Selection &sel) override;

private:
    struct ExtractedParams {
        std::vector<std::tuple<std::string, std::string, std::string, bool>> params;
    };

    const Selection* sel_ = nullptr;
    CallExpr* callExpr_ = nullptr;
    FuncDecl* funcDecl_ = nullptr;
    ExtractedParams extractedParams_;
    std::string indent_;
    std::string resultVarName_;
    bool hasReturnValue_ = false;

    CallExpr* GetCallExpr();

    FuncDecl* GetFuncDecl();

    bool HasReturnValue();

    std::string GetSourceCode(Ptr<Node> node);

    std::string GetReturnTypeString();

    std::string GetParamTypeString(FuncParam* param);

    std::string ReplaceParamsInCode(const std::string &code, const std::map<std::string, std::string> &paramMap);

    std::string GenerateInlineVarName();

    std::string GenerateParamVarName(const std::string &paramName);

    bool IsSimpleArg(Expr* expr);

    bool IsComplexArg(Expr* expr);

    void ExtractParams();

    std::string GetIndent(Position pos);

    std::string TransformFunctionBody(Block* block, const std::map<std::string, std::string> &paramMap);

    std::string TransformReturnStatement(ReturnExpr* returnExpr, const std::map<std::string, std::string> &paramMap);

    void AppendStatementCode(std::ostringstream &bodyCode, Node* stmt,
        const std::map<std::string, std::string> &paramMap);

    Range FindInsertPosition(std::string &indent);

    Range FindInsertPositionFromBlock(Range &callRange, bool &isGlobal, std::string &indent);

    Range FindInsertPositionFromToken(std::string &indent);

    TextEdit InsertParamDecls();

    TextEdit InsertInlineBody();

    TextEdit ReplaceCallWithVar();
};

} // namespace ark

#endif // CANGJIE_LSP_INLINEFUNCTION_H