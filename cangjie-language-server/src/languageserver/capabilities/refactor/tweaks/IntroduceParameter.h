// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_INTRODUCEPARAMETER_H
#define CANGJIE_LSP_INTRODUCEPARAMETER_H

#include "../../../../json-rpc/Protocol.h"
#include "../Tweak.h"
namespace ark {
class IntroduceParameter : public Tweak {
public:
    enum class IntroduceParameterError {
        FAIL_GET_ROOT_EXPR = 2,
        INVALID_EXPR,
        INVALID_CODE_SEGMENT,
        INVALID_SCOPE,
        INVALID_TYPE,
        PUBLIC_DECL_USES_NON_PUBLIC_TYPE
    };

    const std::string Id() const override
    {
        return "IntroduceParameter";
    };

    std::string Title() const override
    {
        return "Introduce expression to parameter";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Selection &sel) override;

    std::optional<Effect> Apply(const Selection &sel) override;

    std::map<std::string, std::string> ExtraOptions() override;

    static Ptr<Cangjie::AST::FuncDecl> GetTargetFunc(const Selection &sel);

    static TextEdit InsertParameter(Cangjie::AST::FuncDecl &funcDecl, std::string &paramName, std::string &typeName);

    static TextEdit ReplaceExprWithParam(const Selection &sel, Range &range, std::string &paramName);
};
} // namespace ark

#endif // CANGJIE_LSP_INTRODUCEPARAMETER_H
