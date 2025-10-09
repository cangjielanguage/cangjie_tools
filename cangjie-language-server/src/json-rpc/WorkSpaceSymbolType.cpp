// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "WorkSpaceSymbolType.h"

namespace ark {
bool FromJSON(const nlohmann::json &params, WorkspaceSymbolParams &reply)
{
    nlohmann::json query = params["query"];
    if (query.is_null()) {
        return false;
    }
    reply.query = query.get<std::string>();
    return true;
}

bool ToJSON(const SymbolInformation &input, nlohmann::json &item)
{
    item["containerName"] = input.containerName;
    item["name"] = input.name;
    item["kind"] = static_cast<int>(input.kind);
    item["location"]["range"]["start"]["line"] = input.location.range.start.line;
    item["location"]["range"]["start"]["character"] = input.location.range.start.column;
    item["location"]["range"]["end"]["line"] = input.location.range.end.line;
    item["location"]["range"]["end"]["character"] = input.location.range.end.column;
    item["location"]["uri"] = input.location.uri.file;
    return true;
}
}