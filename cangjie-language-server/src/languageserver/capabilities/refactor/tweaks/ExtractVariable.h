// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_EXTRACTVARIABLE_H
#define CANGJIE_LSP_EXTRACTVARIABLE_H

#include "../../../../json-rpc/Protocol.h"
#include "../Tweak.h"

namespace ark {
class ExtractVariable : public Tweak {
public:
    enum class ExtractVariableError {
        FAIL_GET_ROOT_EXPR = 2,
        FAIL_MATCH_EXPR,
        INVALID_EXPR,
        INVALID_CODE_SEGMENT
    };

    const std::string Id() const override
    {
        return "ExtractVariable";
    };

    std::string Title() const override
    {
        return "Extract expression to variable";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Selection &sel) override;

    std::optional<Effect> Apply(const Selection &sel) override;

    std::map<std::string, std::string> ExtraOptions() override;

    Range GetExtractionRange(const Selection &sel);

    static TextEdit InsertDeclaration(const Selection &sel, Range &range, std::string &varName);

    static void FindInsertDeclPosition(const Selection &sel, Range &range, Range &insertRange,
        std::string &indent, bool &isGlobal);

    static void FindInsertPositionByScopeName(const Selection &sel, Range &range, Range &insertRange, bool &isGlobal);

    static std::string GetVarModifier(const Selection &sel, Range &range);

    static TextEdit ReplaceExprWithVar(const Selection &sel, Range &range, std::string &varName);

    static void MatchModifier(Cangjie::AST::Node &node, std::string &modifier);

    static void DealMultStatementOnSameLine(
        const Tweak::Selection &sel, const Range &range, int firstToken4CurLine, Range &insertRange);

    static void GetInsertRange(
        const Selection &sel, Range &range, bool isGlobal, Ptr<Block> &block, Range &insertRange);
};
} // namespace ark

#endif // CANGJIE_LSP_EXTRACTVARIABLE_H
