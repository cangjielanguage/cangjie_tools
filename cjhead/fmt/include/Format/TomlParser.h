// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_TOMLPARSER_H
#define CJFMT_TOMLPARSER_H

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <optional>

namespace Cangjie::Format {
class TomlParser {
public:
    using ValueType = std::variant<int, bool, std::string>;

    bool ReadFile(const std::string& filename);
    std::optional<ValueType> GetValue(const std::string& key) const;

private:
    std::map<std::string, std::optional<ValueType>> data;
    std::string Trim(const std::string& str);
    std::optional<ValueType> ParseValue(const std::string& value);
};
} // namespace Cangjie::Format
#endif // CJFMT_TOMLPARSER_H
