// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_INTRODUCECONSTANT_H
#define CANGJIE_LSP_INTRODUCECONSTANT_H

#include "../Tweak.h"

namespace ark {
class IntroduceConstant : public Tweak {
public:
    enum class IntroduceConstantError {
        INVALID_EXPR = 2,
        INVALID_CONST_EXPR,
        PARTIAL_SELECTION
    };

    const std::string Id() const override
    {
        return "IntroduceConstant";
    };

    std::string Title() const override
    {
        return "Introduce expression to constant variable";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Selection &sel) override;

    std::optional<Effect> Apply(const Selection &sel) override;

    std::map<std::string, std::string> ExtraOptions() override;

    static TextEdit InsertDeclaration(const Selection &sel, Range &range, std::string &varName);

    static TextEdit ReplaceExprWithVar(const Selection &sel, Range &range, std::string &varName);
};
} // namespace ark

#endif // CANGJIE_LSP_INTRODUCECONSTANT_H
