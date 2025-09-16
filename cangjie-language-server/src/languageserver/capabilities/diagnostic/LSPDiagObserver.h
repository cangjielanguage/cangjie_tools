// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_DIAGNOSTIC_H
#define LSPSERVER_DIAGNOSTIC_H

#include <iostream>
#include <functional>
#include "cangjie/Basic/DiagnosticEngine.h"
#include "../../common/Callbacks.h"
#include "../../common/PositionResolver.h"

namespace ark {
enum class NeedDiagnostics {
    YES,  /// Diagnostics must be generated for this snapshot.
    NO,   /// Diagnostics must not be generated for this snapshot.
    AUTO, /// Diagnostics must be generated for this snapshot or a subsequent one
};

enum class DiagLSPSeverity {
    ERROR_DIAG = 1,
    WARNING = 2,
    INFO = 3,
    HINT = 4,
    DEFAULT_DIAG = 0
};

class LSPDiagObserver : public Cangjie::DiagnosticHandler {
public:
    explicit LSPDiagObserver(Callbacks *c, Cangjie::DiagnosticEngine &engine);
    void HandleDiagnose(Cangjie::Diagnostic& diagnostic) override;
    ~LSPDiagObserver() override;
private:
    Callbacks *callback = nullptr;
    void FormatDiags(DiagnosticToken &diagToken, SubDiagnostic &subDiag, std::set<char> endPunctuation);
    void AddNoteInfo(Cangjie::Diagnostic &diagnostic, std::vector<DiagnosticRelatedInformation> &relatedInformation);
    void DealMacroDiags(Cangjie::Diagnostic &diagnostic, const DiagnosticToken &token);
};
} // namespace ark

#endif // LSPSERVER_DIAGNOSTIC_H
