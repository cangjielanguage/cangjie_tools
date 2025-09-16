// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Tweak.h"
#include "../../logger/Logger.h"
#include "TweakRegistry.h"

namespace ark {
Tweak::Selection::Selection(ArkAST &arkAst, Range &range,
                            SelectionTree &&selectionTree, std::map<std::string, std::string> extraOptions)
    : arkAst(&arkAst), range(range), selectionTree(std::move(selectionTree))
{
    this->extraOptions = extraOptions;
}

std::vector<std::unique_ptr<Tweak>> Tweak::PrepareTweaks(const Tweak::Selection &selection,
    std::function<bool(const Tweak &)> filter)
{
    std::vector<std::unique_ptr<Tweak>> availableTweaks;
    for (const auto &id : TweakRegistry::AvailableIds()) {
        auto tweak = TweakRegistry::Create(id);
        if (tweak && filter(*tweak) && tweak->Prepare(selection)) {
            availableTweaks.push_back(std::move(tweak));
        }
    }
    return availableTweaks;
}

std::optional<std::unique_ptr<Tweak>> Tweak::PrepareTweak(std::string id, const Tweak::Selection &selection)
{
    for (const auto &tweakId : TweakRegistry::AvailableIds()) {
        if (tweakId != id) {
            continue;
        }
        auto tweak = TweakRegistry::Create(id);
        if (!tweak->Prepare(selection)) {
            return std::nullopt;
        }
        return std::move(tweak);
    }
    return std::nullopt;
}
} // namespace ark