// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Regexp.h"
#include "Connection.h"

#include <optional>
#include <regex>
#include <string>
#include <unordered_map>

namespace sqldb {

static std::optional<bool> regexp(std::optional<std::string_view> Regex, std::optional<std::string_view> Text)
{
    thread_local std::unordered_map<std::string, std::regex> RegexCache;
    if (Regex && Text) {
        auto [ElemIt, Inserted] = RegexCache.try_emplace({Regex->begin(), Regex->end()}, Regex->begin(), Regex->end());
        return std::regex_search(Text->begin(), Text->end(), ElemIt->second);
    }
    return std::nullopt;
}

void registerRegexpFunction(Connection &DB) { DB.scalar("regexp", regexp); }

} // namespace sqldb