// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "TweakRegistry.h"
#include "../../logger/Logger.h"
#include "Tweak.h"
#include "tweaks/ExtractFunction.h"
#include "tweaks/ExtractVariable.h"
#include "tweaks/IntroduceConstant.h"

namespace ark {
#define REGISTER_TWEAK(TweakClass) \
    namespace { \
    struct TweakClass##Registrar { \
        TweakClass##Registrar() noexcept { \
            TweakRegistry::RegisterTweak(#TweakClass, []{ \
                return std::make_unique<TweakClass>(); \
            }); \
        } \
    }; \
    [[maybe_unused]] TweakClass##Registrar TweakClass##_registrar; \
    }

REGISTER_TWEAK(ExtractFunction)
REGISTER_TWEAK(ExtractVariable)
REGISTER_TWEAK(IntroduceConstant)

std::unordered_map<std::string, TweakRegistry::Creator>& TweakRegistry::GetRegistry()
{
    static std::unordered_map<std::string, TweakRegistry::Creator> registry;
    return registry;
}

void TweakRegistry::RegisterTweak(const std::string &id, TweakRegistry::Creator creator)
{
    GetRegistry().emplace(id, std::move(creator));
}

std::unique_ptr<Tweak> TweakRegistry::Create(const std::string& id)
{
    auto &registry = GetRegistry();
    if (auto it = registry.find(id); it != registry.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> TweakRegistry::AvailableIds()
{
    std::vector<std::string> ids;
    for (const auto &[id, _] : GetRegistry()) {
        ids.push_back(id);
    }
    return ids;
}
} // namespace ark