// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_WORKSPACESYMBOLTYPE_H
#define LSPSERVER_WORKSPACESYMBOLTYPE_H
#include "Common.h"
#include "nlohmann/json.hpp"

/**
 * According to the language service protocol to create structure
 * see https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#baseProtocol
 */
namespace ark {
enum class SymbolKind {
    FILE = 1,
    PACKAGE = 4,
    CLASS = 5,
    PROPERTY = 7,
    CONSTRUCTOR = 9,
    ENUM = 10,
    INTERFACE_DECL = 11,
    FUNCTION = 12,
    VARIABLE = 13,
    BOOLEAN = 17,
    OBJECT = 19,
    NULL_KIND = 21,
    ENUMMEMBER = 22,
    STRUCT = 23,
    OPERATOR = 25,
};

struct WorkspaceSymbolParams {
    std::string query {};
};

struct SymbolInformation {
    std::string name {};
    SymbolKind kind {SymbolKind::NULL_KIND};
    Location location {};
    std::string containerName {};

    bool operator<(const ark::SymbolInformation &right) const
    {
        return std::tie(this->location, this->kind, this->name, this->containerName) <
               std::tie(right.location, right.kind, right.name, right.containerName);
    }

    bool operator==(const ark::SymbolInformation &right) const
    {
        return std::tie(this->location, this->kind, this->name, this->containerName) ==
               std::tie(right.location, right.kind, right.name, right.containerName);
    }

    bool operator!=(const ark::SymbolInformation &right) const
    {
        return !(*this == right);
    }
};

bool FromJSON(const nlohmann::json &params, WorkspaceSymbolParams &reply);
bool ToJSON(const SymbolInformation &params, nlohmann::json &item);
}

#endif // LSPSERVER_WORKSPACESYMBOLTYPE_H
