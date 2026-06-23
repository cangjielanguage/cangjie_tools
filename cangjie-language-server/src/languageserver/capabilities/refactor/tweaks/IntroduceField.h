// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_INTRODUCEFIELD_H
#define CANGJIE_LSP_INTRODUCEFIELD_H

#include "../../../../json-rpc/Protocol.h"
#include "../Tweak.h"

namespace ark {
class IntroduceField : public Tweak {
public:
    enum class IntroduceFieldError {
        FAIL_GET_ROOT_EXPR = 2,
        INVALID_EXPR,
        INVALID_CODE_SEGMENT,
        INVALID_SCOPE,
        INVALID_TYPE,
        INVALID_CONST_INITIALIZER,
        INVALID_LET_PATTERN_DESTRUCTOR,
        INVALID_IMMUTABLE_STRUCT_MEMBER_FIELD_ASSIGNMENT
    };

    const std::string Id() const override
    {
        return "IntroduceField";
    };

    std::string Title() const override
    {
        return "Introduce expression to field";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Selection &sel) override;

    std::optional<Effect> Apply(const Selection &sel) override;

    std::map<std::string, std::string> ExtraOptions() override;

    static Ptr<Cangjie::AST::FuncDecl> GetTargetFunc(const Selection &sel);

    static bool IsSupportedTargetFunc(Cangjie::AST::FuncDecl &funcDecl);

    static Ptr<Cangjie::AST::InheritableDecl> GetOwnerDecl(const Selection &sel);

    static bool IsMemberFieldTarget(Cangjie::AST::FuncDecl &funcDecl);

    static bool IsStaticFieldTarget(Cangjie::AST::FuncDecl &funcDecl);

    static bool IsImmutableStructMemberFieldAssignment(
        const Selection &sel, Range &range, const std::string &typeName, Cangjie::AST::FuncDecl &funcDecl);

    static std::optional<Range> GetFieldInsertRange(
        const Selection &sel, Range &range, Cangjie::AST::FuncDecl &funcDecl);

    static Range GetAssignInsertRange(const Selection &sel, Range &range);

    static TextEdit InsertFieldDeclaration(const Selection &sel, Range &range, std::string &fieldName,
        std::string &typeName, Cangjie::AST::FuncDecl &funcDecl);

    static TextEdit InsertFieldAssignment(
        const Selection &sel, Range &range, std::string &fieldName, Cangjie::AST::FuncDecl &funcDecl);

    static TextEdit ReplaceExprWithField(
        const Selection &sel, Range &range, std::string &fieldName, Cangjie::AST::FuncDecl &funcDecl);
};
} // namespace ark

#endif // CANGJIE_LSP_INTRODUCEFIELD_H
