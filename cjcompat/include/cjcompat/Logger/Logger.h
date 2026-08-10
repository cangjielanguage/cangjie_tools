// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

#include <vector>

#include "cangjie/AST/Node.h"
#include "cangjie/AST/Symbol.h"

#include "cjcompat/CheckersAndRules/Rule.h"
#include "cjcompat/CjoLoader/CjoLoader.h"

class Logger {
public:
    Logger(bool checkAPI, bool checkABI) : checkAPI(checkAPI), checkABI(checkABI)
    {
    }
    struct Entry {
        RuleKind rule;
        std::vector<const Cangjie::AST::Node*> nodes;
    };

    void LogIfFalse(RuleKind rule, std::vector<const Cangjie::AST::Node*>&& nodes, bool result);

    void Print(CjoLoader& loader1, CjoLoader& loader2) const;

    const std::vector<Entry>& GetEntries() const;

    /*******************/
    /***** Options *****/
    /*******************/

    /** @brief Check for API rules. */
    const bool checkAPI;
    /** @brief Check for ABI rules. */
    const bool checkABI;

private:
    std::vector<Entry> entries;
};

#endif // LOGGER_LOGGER_H