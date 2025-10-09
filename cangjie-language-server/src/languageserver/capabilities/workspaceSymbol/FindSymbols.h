// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_FINDSYMBOLS_H
#define LSPSERVER_FINDSYMBOLS_H

#include <vector>
#include "../../../json-rpc/Transport.h"
#include "../../../json-rpc/WorkSpaceSymbolType.h"
#include "../../index/MemIndex.h"

namespace ark {
namespace lsp {

std::vector<SymbolInformation> GetWorkspaceSymbols(const std::string &query);

}
} // namespace ark
#endif // LSPSERVER_FINDSYMBOLS_H
