// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "TweakRule.h"
#include "../../logger/Logger.h"

namespace ark {
// 1. selected.start != selected.end
// 2. root != null
// 3. contain a SelectionTree::Complete node
bool TweakRule::CommonCheck(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions)
{
    if (!sel.arkAst || !sel.arkAst->file || !sel.arkAst->sourceManager) {
        return false;
    }
    if (sel.range.start.fileID == sel.range.end.fileID && sel.range.start == sel.range.end) {
        return false;
    }
    auto root = sel.selectionTree.root();
    if (!root || !root->node) {
        return false;
    }
    bool isValid = true;
    bool containComplete = false;
    SelectionTree::Walk(root, [&isValid, &containComplete]
        (const SelectionTree::SelectionTreeNode *node) {
            if (!node->node) {
                isValid = false;
                return SelectionTree::WalkAction::STOP_NOW;
            }

            if (node->selected == SelectionTree::Selection::Complete) {
                containComplete = true;
            }

            return SelectionTree::WalkAction::WALK_CHILDREN;
        });
    if (!(isValid && containComplete)) {
        extraOptions.insert(std::make_pair("ErrorCode",
            std::to_string(static_cast<int>(TweakRule::TweakError::TWEAK_FAIL))));
    }
    return isValid && containComplete;
}

bool TweakRuleEngine::CheckRules(const Tweak::Selection &sel, std::map<std::string, std::string> &extraOptions)
{
    if (rules.empty()) {
        return true;
    }
    return TweakRule::CommonCheck(sel, extraOptions)
           && std::all_of(rules.begin(), rules.end(),
                  [&](const auto& rule) { return rule->Check(sel, extraOptions); });
}
} // namespace ark