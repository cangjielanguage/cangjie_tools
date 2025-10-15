// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef EXCLUDE_RULE_H
#define EXCLUDE_RULE_H

#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include <exception>
#include "cangjie/Utils/FileUtil.h"

namespace Cangjie::CodeCheck {
class ExcludeRule {
public:
    enum class ExcludeRuleType {
        IGNORE = 0,
        INCLUDE
    };
    enum class ExcludeRuleTarget {
        FILE_AND_DIR = 0,
        DIRECTORY
    };
    ExcludeRule(std::string srcFileDir, std::string exclude);
    ~ExcludeRule() = default;
    const ExcludeRuleType GetRuleType() const;
    const ExcludeRuleTarget GetRuleTarget() const;
    bool IsMatched(const std::string &target);
    bool isValid = true;

private:
    void Preprocess(std::string &regexStr);
    void MatchSignModification(std::string &regexStr);

    ExcludeRuleType ruleType = ExcludeRuleType::IGNORE;
    ExcludeRuleTarget ruleTarget = ExcludeRuleTarget::FILE_AND_DIR;
    std::string excludeRegexStr;
    std::regex excludeRegex;
    bool nonTrailSlash = false;
};
} // namespace Cangjie::CodeCheck::ExcludeRule

#endif // EXCLUDE_RULE_H