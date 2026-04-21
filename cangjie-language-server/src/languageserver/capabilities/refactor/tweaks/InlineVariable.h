// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_INLINEVARIABLE_H
#define CANGJIE_LSP_INLINEVARIABLE_H

#include "../Tweak.h"

namespace ark {

class InlineVariable : public Tweak {
public:
    enum class InlineVariableError {
        NOT_VAR_DECL_OR_REF = 2,
        NO_INIT_EXPR,
        VAR_MODIFIED,
        MEMBER_VAR,
        NO_TARGET
    };

    const std::string Id() const override
    {
        return "InlineVariable";
    }

    std::string Title() const override
    {
        return "Inline variable";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Selection &sel) override;

    std::optional<Effect> Apply(const Selection &sel) override;

private:
    VarDecl* GetVarDecl(const Selection &sel);

    std::string GetSourceCode(Ptr<Node> node, const Selection &sel);

    bool NeedsParentheses(Expr* initExpr, const Selection &sel);

    bool IsSimpleExpr(Expr* expr);

    TextEdit ReplaceRefWithInitExpr(const Selection &sel, RefExpr* refExpr, const std::string &initExprCode);
};

} // namespace ark

#endif // CANGJIE_LSP_INLINEVARIABLE_H