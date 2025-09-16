// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "LSPDiagObserver.h"
#include "../../CompilerCangjieProject.h"

namespace ark {
using namespace Cangjie;
const std::unordered_set<DiagKind> IMPORT_FIX_KIND = {DiagKind::sema_undeclared_identifier,
    DiagKind::sema_undeclared_type_name, DiagKind::macro_undeclared_identifier};

const std::unordered_set<DiagKindRefactor> IMPORT_FIX_RKIND = {DiagKindRefactor::parse_expected_decl,
    DiagKindRefactor::lex_expected_identifier, DiagKindRefactor::sema_undeclared_identifier};

LSPDiagObserver::LSPDiagObserver(Callbacks *c, Cangjie::DiagnosticEngine &engine)
    : Cangjie::DiagnosticHandler(engine, Cangjie::DiagHandlerKind::LSP_HANDLER), callback(c)
{
    diag.SetIsDumpErrCnt(false);
    diag.SetIsEmitter(false);
}
LSPDiagObserver::~LSPDiagObserver() = default;
DiagLSPSeverity GetSeverity(DiagSeverity l)
{
    switch (l) {
        case DiagSeverity::DS_HINT:
            return DiagLSPSeverity::HINT;   // Hint
        case DiagSeverity::DS_NOTE:
            return DiagLSPSeverity::INFO;   // Info
        case DiagSeverity::DS_WARNING:
            return DiagLSPSeverity::WARNING;   // Warning
        case DiagSeverity::DS_ERROR:
            return DiagLSPSeverity::ERROR_DIAG;   // Error
        default:
            return DiagLSPSeverity::DEFAULT_DIAG;   // default
    }
}

void SetDiagMessage(Cangjie::Diagnostic &diagnostic, DiagnosticToken &diagToken)
{
    if (diagnostic.rKind == DiagKindRefactor::module_read_file_to_buffer_failed) {
        diagToken.message = "Empty file can not be compiled";
    } else if (diagnostic.rKind == DiagKindRefactor::sema_mismatched_types ||
               diagnostic.rKind == DiagKindRefactor::sema_mismatched_types_because) {
        diagToken.message = diagnostic.GetErrorMessage() + " " + diagnostic.mainHint.str;
    } else {
        diagToken.message = diagnostic.GetErrorMessage();
    }
}

void LSPDiagObserver::HandleDiagnose(Cangjie::Diagnostic &diagnostic)
{
    // LSP client can't process negative
    if (!diagnostic.IsValid() || diagnostic.rKind == DiagKindRefactor::package_multiple_package_declarations ||
        diagnostic.rKind == DiagKindRefactor::package_unsupport_save) {
        return;
    }

    // Blocks crashes caused by binary mismatches in Release
#ifndef DEBUG
    if (diagnostic.rKind == DiagKindRefactor::module_version_not_identical ||
        diagnostic.rKind == DiagKindRefactor::module_loaded_ast_failed) {
        if (!MessageHeaderEndOfLine::GetIsDeveco()) {
            // Ensure that the server can exit normally.
            CompilerCangjieProject::GetInstance()->isIdentical = false;
            // Prompt the user through the client.
            callback->ReportCjoVersionErr(diagnostic.errorMessage);
        }
    }
#endif

    if (!diagnostic.isRefactor) {
        if (HasPrevDiag(diagnostic.start, diagnostic.diagMessage)) {
            return;
        }
        SetPrevDiag(diagnostic.start, diagnostic.diagMessage);
    }

    if (diagnostic.diagSeverity == DiagSeverity::DS_ERROR) {
        diag.IncreaseErrorCount(diagnostic.diagCategory);
    }
    DiagnosticToken diagToken;
    diagToken.severity = static_cast<int>(GetSeverity(diagnostic.diagSeverity));
    auto start = diagnostic.GetBegin();
    auto end = diagnostic.GetEnd();
    diagToken.range = {{start.fileID, start.line - 1, start.column - 1}, {end.fileID, end.line - 1, end.column - 1}};
    diagToken.source = "Cangjie";
    diagToken.code = static_cast<int>(diagnostic.GetDiagKind());
    SetDiagMessage(diagnostic, diagToken);
    diagToken.category = diagnostic.GetDiagKind();
    if (diagnostic.rKind == DiagKindRefactor::sema_unused_import) {
        diagToken.tags = {1};
        diagToken.severity = static_cast<int>(DiagLSPSeverity::HINT);
    }

    // diagToken.category is enhanced features of clangd, GetCategory(diagnostic.diagCategory) can get it
    std::string filePath = diag.GetSourceManager().GetSource(diagnostic.GetBegin().fileID).path;

    // provider auto import quick fix
    bool isRefactor = (!diagnostic.isRefactor && IMPORT_FIX_KIND.count(diagnostic.kind))
        || (diagnostic.isRefactor && IMPORT_FIX_RKIND.count(diagnostic.rKind));
    if (isRefactor) {
        diagToken.diaFix->isAutoImport = true;
    }
    
    // Add diagnostic details
    // If the position is incorrect, the error cause is located in the current position.
    std::vector<DiagnosticRelatedInformation> relatedInformation;
    std::set<char> endPunctuation = {',', '.', '?', '!', ':'};
    for (auto &subDiag : diagnostic.subDiags) {
        if (subDiag.mainHint.range.begin.column < 0) {
            FormatDiags(diagToken, subDiag, endPunctuation);
            continue;
        }

        DiagnosticRelatedInformation info;
        info.message = subDiag.subDiagMessage;
        info.location.range = {{subDiag.mainHint.range.begin.fileID, subDiag.mainHint.range.begin.line - 1,
                                   subDiag.mainHint.range.begin.column - 1},
            {subDiag.mainHint.range.end.fileID, subDiag.mainHint.range.end.line - 1,
                subDiag.mainHint.range.end.column - 1}};
        info.location.uri.file =
            URI::URIFromAbsolutePath(diag.GetSourceManager().GetSource(subDiag.mainHint.range.begin.fileID).path)
                .ToString();
        relatedInformation.emplace_back(info);
    }

    // add note Info
    AddNoteInfo(diagnostic, relatedInformation);

    diagToken.relatedInformation.emplace(relatedInformation);
    callback->UpdateDiagnostic(filePath, diagToken);
    DealMacroDiags(diagnostic, diagToken);
}

void LSPDiagObserver::AddNoteInfo(Cangjie::Diagnostic &diagnostic,
    std::vector<DiagnosticRelatedInformation> &relatedInformation)
{
    for (auto &note : diagnostic.notes) {
        DiagnosticRelatedInformation info;
        info.message = note.GetErrorMessage();
        auto noteStart = note.GetBegin();
        auto noteEnd = note.GetEnd();
        auto noteFilePath = diag.GetSourceManager().GetSource(noteStart.fileID).path;
        if (FileUtil::FileExist(noteFilePath)) {
            info.location.range = {{noteStart.fileID, noteStart.line - 1, noteStart.column - 1},
                {noteEnd.fileID, noteEnd.line - 1, noteEnd.column - 1}};
            info.location.uri.file =
                URI::URIFromAbsolutePath(diag.GetSourceManager().GetSource(noteStart.fileID).path).ToString();
            relatedInformation.emplace_back(info);
            continue;
        }
        std::size_t pos = noteFilePath.find_last_of(CONSTANTS::FILE_SEPARATOR);
        if (pos != std::string::npos) {
            auto notePackage = noteFilePath.substr(0, pos);
            auto fileName = noteFilePath.substr(pos + 1);
            auto pkgPath = CompilerCangjieProject::GetInstance()->GetPathFromPkg(notePackage);
            if (!pkgPath.empty()) {
                info.location.uri.file =
                    URI::URIFromAbsolutePath(pkgPath + CONSTANTS::FILE_SEPARATOR + fileName).ToString();
                info.location.range = {{noteStart.fileID, noteStart.line - 1, noteStart.column - 1},
                    {noteEnd.fileID, noteEnd.line - 1, noteEnd.column - 1}};
            }
        }
        relatedInformation.emplace_back(info);
    }
}

void LSPDiagObserver::FormatDiags(DiagnosticToken &diagToken, SubDiagnostic &subDiag, std::set<char> endPunctuation)
{
    if (!diagToken.message.empty() && RTrim(diagToken.message).back() == '.') {
        diagToken.message = RTrim(diagToken.message);
        if (!diagToken.message.empty()) {
            diagToken.message.pop_back();
        }
    }
    bool containEndChar = endPunctuation.find(Trim(diagToken.message).back()) != endPunctuation.end();
    diagToken.message = containEndChar ? (diagToken.message + " " + subDiag.subDiagMessage)
                                       : (diagToken.message + ", " + subDiag.subDiagMessage);
}

void LSPDiagObserver::DealMacroDiags(Cangjie::Diagnostic &diagnostic, const DiagnosticToken &token)
{
    std::string filePath = diag.GetSourceManager().GetSource(diagnostic.GetBegin().fileID).path;
    if (FileUtil::GetFileExtension(filePath) != "macrocall" || diagnostic.subDiags.empty()) {
        return;
    }
    DiagnosticToken diagToken = token;
    for (auto &subDiag : diagnostic.subDiags) {
        std::string subDiagPath =
            diag.GetSourceManager().GetSource(subDiag.mainHint.range.begin.fileID).path;
        if (FileUtil::GetFileExtension(subDiagPath) == "cj") {
            diagToken.range = {subDiag.mainHint.range.begin, subDiag.mainHint.range.end};
            diagToken.range = TransformFromChar2IDE(diagToken.range);
            callback->UpdateDiagnostic(subDiagPath, diagToken);
            return;
        }
    }
}
} // namespace ark
