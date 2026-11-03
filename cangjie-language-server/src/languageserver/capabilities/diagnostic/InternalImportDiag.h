// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_INTERNAL_IMPORT_DIAG_H
#define LSPSERVER_INTERNAL_IMPORT_DIAG_H

#include "../../common/Callbacks.h"
#include "cangjie/Modules/ImportManager.h"

namespace ark {
class InternalImportDiag {
public:
    // Recheck internal imports using resolved AST references only; do not change semantic/export caches.
    static void Refresh(const Cangjie::AST::Package &package, Cangjie::ImportManager &imports,
        Cangjie::DiagnosticEngine &diag, Callbacks &callback, bool noSubPkg);
};
} // namespace ark

#endif // LSPSERVER_INTERNAL_IMPORT_DIAG_H
