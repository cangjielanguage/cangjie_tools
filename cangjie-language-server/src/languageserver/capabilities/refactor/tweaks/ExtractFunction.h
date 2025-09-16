// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_EXTRACTFUNCTION_H
#define CANGJIE_LSP_EXTRACTFUNCTION_H

#include "../Tweak.h"

namespace ark {

struct ExtractedFunction {
    struct Param {
        // param name
        std::string name;

        // param type
        std::string type;

        // rename param name
        std::optional<std::string> newName;

        bool isGeneric = false;

        bool operator==(const Param& other) const
        {
            return name == other.name && type == other.type;
        }

        bool operator!=(const Param& other) const
        {
            return !(*this == other);
        }

        bool operator<(const Param& other) const
        {
            return std::tie(name, type) < std::tie(other.name, other.type);
        }
    };

    struct ReturnValue {
        // return name
        std::string name;

        // the function whether need add return statement at the last line of body
        bool addReturnOnFuncBody = false;

        // need define a variable to receive the return value,
        // or use the existing variable to receive the return value
        bool needDeclVar = false;
    };

    // function name
    std::string name = "extracted";

    // param list
    std::set<Param> params;

    // Return type (node) of the function
    std::optional<ReturnValue> returnValue;

    // function body
    std::string body;

    // is static function
    bool isStatic = false;

    bool isConst = false;

    bool isMut = false;

    // selected range
    Range replacedRange;

    std::string GetFunctionDeclaration() const;
};

class ExtractFunction : public Tweak {
public:
    enum class ExtractFunctionError {
        INVALID_CODE_SEGMENT = 2,
        PARTIAL_SELECTION,
        MULTI_RETURN_VALUE,
        PARTIAL_IF_EXPR,
        PARTIAL_JUMP_EXPR,
        MULTI_EXIT_POINT,
        GLOBAL_MEMBER_VAR,
        CONTAIN_MEMBER_VAR_INIT,
        PARTIAL_TRY_EXPR,
        PARTIAL_MATCH_EXPR
    };

    const std::string Id() const override
    {
        return "ExtractFunction";
    };

    std::string Title() const override
    {
        return "Extract to function";
    }

    std::string Kind() const override
    {
        return CodeAction::REFACTOR_KIND;
    }

    bool Prepare(const Selection &sel) override;

    std::optional<Effect> Apply(const Selection &sel) override;

private:
    void GetExtractedFunction(const Tweak::Selection &sel, ExtractedFunction &function);

    void GetParam(ExtractedFunction& function, Cangjie::AST::RefExpr* refExpr, const Tweak::Selection &sel);

    void GetFunctionParamsList(const Tweak::Selection &sel, ExtractedFunction &function);

    void GetFunctionReturnValue(const Tweak::Selection &sel, ExtractedFunction &function);

    void GetFunctionBody(const Tweak::Selection &sel, ExtractedFunction &function);

    void GetFunctionModifier(const Tweak::Selection &sel, ExtractedFunction &function);

    std::string AssignParam2MutVarAndRemove(const Tweak::Selection &sel, ExtractedFunction &function);

    void AddMutParamVariable(std::string &mutParams, Cangjie::AST::AssignExpr* assignExpr,
        ExtractedFunction &function, const Tweak::Selection &sel);

    bool ExistSameNameParam(const std::set<ExtractedFunction::Param>& params, const std::string& name);

    TextEdit InsertDeclaration(const Tweak::Selection &sel, ExtractedFunction &function);

    TextEdit ReplaceBlockWithCall(const Tweak::Selection &sel, ExtractedFunction &function);
};
} // namespace ark

#endif // CANGJIE_LSP_EXTRACTFUNCTION_H
