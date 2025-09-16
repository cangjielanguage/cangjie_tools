// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_TWEAKREGISTRY_H
#define CANGJIE_LSP_TWEAKREGISTRY_H

#include <functional>
#include <string>
#include "Tweak.h"

namespace ark {

class TweakRegistry {
public:
    using Creator = std::function<std::unique_ptr<Tweak>()>;

    TweakRegistry() = delete;

    static void RegisterTweak(const std::string &id, Creator creator);

    static std::unique_ptr<Tweak> Create(const std::string &id);

    static std::vector<std::string> AvailableIds();

private:
    static std::unordered_map<std::string, Creator>& GetRegistry();
};
} // namespace ark

#endif // CANGJIE_LSP_TWEAKREGISTRY_H
