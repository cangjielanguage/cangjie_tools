// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_TWEAK_H
#define CANGJIE_LSP_TWEAK_H

#include "../../selection/SelectionTree.h"
#include "../../../json-rpc/CompletionType.h"
#include "../../ArkAST.h"

namespace ark {

/**
 * The interface base for refactoring actions.
 * To implement a new tweak use the following pattern:
 * class MyTweak : public Tweak {
 * };
 * REGISTER_TWEAK(MyTweak);
 */
class Tweak {
public:
    // The Input for prepare and apply tweaks.
    struct Selection {
        Selection(ArkAST &arkAst, Range &range, SelectionTree &&selectionTree,
            std::map<std::string, std::string> extraOptions);

        // The ArkAst of active file.
        ArkAST *arkAst;

        // The range of selected text.
        Range &range;

        // The AST Tree of were selected.
        SelectionTree selectionTree;

        std::map<std::string, std::string> extraOptions;
    };

    struct Effect {
        // A message to be displayed to the user.
        std::optional<std::string> showMessage;

        // all changes
        std::map<std::string, std::vector<TextEdit>> applyEdits;

        // the change text whether format
        bool format = true;

        static Effect ShowMessage(std::string message)
        {
            Effect E;
            E.showMessage = std::string(message);
            return E;
        }
    };

    std::map<std::string, std::string> extraOptions;

    virtual ~Tweak() = default;

    // A unique id of the action,  it is always equal to the name of the class registry tweak.
    virtual const std::string Id() const = 0;

    // check the tweak whether is valid
    virtual bool Prepare(const Selection &inputs) = 0;

    // apply the tweak
    virtual std::optional<Effect> Apply(const Selection &inputs) = 0;

    virtual std::string Title() const = 0;

    virtual std::string Kind() const = 0;

    virtual std::map<std::string, std::string> ExtraOptions()
    {
        return extraOptions;
    };

    // check all tweaks
    static std::vector<std::unique_ptr<Tweak>> PrepareTweaks(const Tweak::Selection &selection,
        std::function<bool(const Tweak &)> filter);

    // Calls prepare() on the tweak with a given ID.
    // If prepare() returns false, returns an error.
    // If prepare() returns true, returns the corresponding tweak.
    static std::optional<std::unique_ptr<Tweak>> PrepareTweak(std::string id, const Tweak::Selection &selection);
};
} // namespace ark

#endif // CANGJIE_LSP_TWEAK_H