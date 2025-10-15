// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ExcludeRule.h"

using namespace Cangjie::CodeCheck;

static void ClearSpaces(std::string &str)
{
    if (str.find_first_not_of(" ") == std::string::npos) {
        str = "";
        return;
    }
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
}

static void ReplaceAllSafely(std::string &str, const std::string src, const std::string dst)
{
    auto index = str.find(src);
    while (index != std::string::npos) {
        str.replace(index, src.length(), dst);
        index = str.find(src);
    }
}

static void ReplaceAllUnsafely(std::string &str, const std::string src, const std::string dst)
{
    std::string newStr = "";
    auto index = str.find(src);
    while (index != std::string::npos) {
        str.replace(index, src.length(), dst);
        newStr += str.substr(0, index + dst.length());
        str = str.substr(index + dst.length());
        index = str.find(src);
    }
    newStr += str;
    str = std::move(newStr);
}

static void ReplaceAllButLastUnsafely(std::string &str, const std::string src, const std::string dst)
{
    std::string newStr = "";
    auto index = str.find(src);
    while ((index != std::string::npos) && (index != str.length() - src.length())) {
        str.replace(index, src.length(), dst);
        newStr += str.substr(0, index + dst.length());
        str = str.substr(index + dst.length());
        index = str.find(src);
    }
    newStr += str;
    str = std::move(newStr);
}

ExcludeRule::ExcludeRule(std::string srcFileDir, std::string exclude)
{
    ReplaceAllSafely(srcFileDir, "\\", "/");
    ReplaceAllSafely(exclude, "\\", "/");
    Preprocess(exclude);
    MatchSignModification(exclude);
    if (srcFileDir.back() != '/') {
        srcFileDir += "/";
    }
    this->excludeRegexStr = srcFileDir + exclude;
#ifdef _WIN32
    ReplaceAllSafely(this->excludeRegexStr, "/", "\\\\");
#endif
    std::regex reg(this->excludeRegexStr);
    this->excludeRegex = reg;
}

const ExcludeRule::ExcludeRuleType ExcludeRule::GetRuleType() const
{
    return this->ruleType;
}

const ExcludeRule::ExcludeRuleTarget ExcludeRule::GetRuleTarget() const
{
    return this->ruleTarget;
}

bool ExcludeRule::IsMatched(const std::string &target)
{
    return std::regex_match(target, this->excludeRegex);
}

/*
 * Preprocess regexStr.
 * After this step, regexStr will:
 * 1. Ignore leading and trailing spaces;
 * 2. Ignore comment with #;
 * 3. Check if ! exists, and set type;
 * 4. Check .. and return exception;
 * 5. Check . and /, fix to correct version.
 */
void ExcludeRule::Preprocess(std::string &regexStr)
{
    if (regexStr == ".") {
        regexStr = "";
        return;
    }
    ClearSpaces(regexStr);

    if (regexStr.empty()) {
        isValid = false;
    }

    // Ignore comment.
    if (regexStr.front() == '#') {
        isValid = false;
    }

    // .. should be avoid.
    if (regexStr.find("..") != std::string::npos) {
        isValid = false;
    }

    // Get rule type.
    if (regexStr.front() == '!') {
        regexStr.erase(0, 1);
        ClearSpaces(regexStr);
        if (regexStr.empty()) {
            isValid = false;
        }
        this->ruleType = ExcludeRuleType::INCLUDE;
    } else {
        this->ruleType = ExcludeRuleType::IGNORE;
    }

    // Proceed . and /
    if ((regexStr.find("/") != std::string::npos) && (regexStr.find("/") < regexStr.size() - 1)) {
        this->nonTrailSlash = true;
    }
    ReplaceAllSafely(regexStr, "/./", "/");
    if (regexStr.find("./") == 0) {
        regexStr.erase(0, 1);
    }
    ReplaceAllSafely(regexStr, "//", "/");
    ReplaceAllUnsafely(regexStr, ".", "\\.");
    return;
}

/*
 * Proceed match signs.
 * After this step, regexStr will:
 * 1. Match *, ** and ?;
 * 2. Match directory.
 */
void ExcludeRule::MatchSignModification(std::string &regexStr)
{
    // Proceed *, ** and ?
    std::string newRegexStr = "";
    size_t index = 0;
    while (index < regexStr.size()) {
        if (regexStr[index] == '*') {
            if ((index < regexStr.size() - 1) && (regexStr[index + 1] == '*')) {
                newRegexStr += ".*";
                ++index;
            } else {
                newRegexStr += "[^/]*";
            }
        } else if (regexStr[index] == '?') {
            newRegexStr += "[^/]";
        } else {
            newRegexStr += regexStr[index];
        }
        ++index;
    }
    regexStr = std::move(newRegexStr);
    ReplaceAllButLastUnsafely(regexStr, ".*/", "(.*/)?");

    // Proceed /
    if (!this->nonTrailSlash) {
        regexStr = "(.*/)?" + regexStr;
    } else if (regexStr.front() == '/') {
        regexStr.erase(0, 1);
    }
    if (regexStr.back() == '/') {
        regexStr += ".*";
        this->ruleTarget = ExcludeRuleTarget::DIRECTORY;
    } else {
        regexStr += "(/.*)?";
        this->ruleTarget = ExcludeRuleTarget::FILE_AND_DIR;
    }

    return;
}
