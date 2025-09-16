// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_SYSCAPCHECK_H
#define LSPSERVER_SYSCAPCHECK_H

#include <unordered_map>
#include <unordered_set>
#include "cangjie/AST/Node.h"

namespace ark {
using SysCapSet = std::unordered_set<std::string>;
class SyscapCheck {
public:
    SyscapCheck() = default;
    explicit SyscapCheck(const std::string& moduleName);
    void SetIntersectionSet(const std::string& moduleName);
    static void ParseCondition(const std::unordered_map<std::string, std::string>& passedWhenKeyValue);
    static void ParseJsonFile(const std::vector<uint8_t>& in);
    // if check node has syscap and not match, return false
    bool CheckSysCap(Ptr<Cangjie::AST::Node> node);
    bool CheckSysCap(Ptr<Cangjie::AST::Node> node, bool& matchSyscap) const;
    bool CheckSysCap(Ptr<Cangjie::AST::Decl> decl) const;
    bool CheckSysCap(const Cangjie::AST::Decl& decl) const;
    bool CheckSysCap(const std::string& syscapName);
    SysCapSet intersectionSet;
    static std::unordered_map<std::string, SysCapSet> module2SyscapsMap;
};

} // namespace ark

#endif // LSPSERVER_SYSCAPCHECK_H