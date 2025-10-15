// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_ARKLANGUAGESERVER_H
#define LSPSERVER_ARKLANGUAGESERVER_H

#include <iostream>
#include <sstream>
#include <memory>
#include <atomic>
#include <vector>
#include <mutex>
#include <cstdint>
#include "../json-rpc/Transport.h"
#include "ArkServer.h"
#include "../json-rpc/Protocol.h"
#include "DocCache.h"
#include "CompilerCangjieProject.h"
#include "capabilities/semanticHighlight/SemanticHighlightImpl.h"
#include "logger/Logger.h"
#include "capabilities/diagnostic/LSPDiagObserver.h"
#include "common/Utils.h"
#include "common/Constants.h"
#include "index/IndexDatabase.h"
#include "index/BackgroundIndexDB.h"

namespace ark {
class ArkLanguageServer : public Callbacks {
public:
    explicit ArkLanguageServer(Transport &transport, Environment environmentVars, lsp::IndexDatabase *db);

    ~ArkLanguageServer() override;

    LSPRet Run() const;

    void RemoveDiagOfCurPkg(const std::string &dirName) override;

    std::vector<DiagnosticToken> GetDiagsOfCurFile(std::string) override;

    void UpdateDiagnostic(std::string file, DiagnosticToken diagToken) override;

    std::string GetContentsByFile(const std::string &file) override;

    std::int64_t GetVersionByFile(const std::string &file) override;

    bool NeedReParser(const std::string &file) override;

    void UpdateDocNeedReparse(const std::string &file, int64_t version, bool needReParser) override;

    void AddDocWhenInitCompile(const std::string &file) override;

    void RemoveDocByFile(const std::string &file) override;

    bool WhetherSupportVersionInDiag() const;

    void WrapClientWatchedFiles(std::vector<FileWatchedEvent> &changes, const DidChangeWatchedFilesParam &params) const;

    void ReadyForDiagnostics(std::string, std::int64_t, std::vector<DiagnosticToken>) override;

    void ReportCjoVersionErr(std::string message) override;

    void PublishCompletionTip(const CompletionTip &params) override;

    lsp::IndexDatabase *GetIndexDB() const
    {
        return arkIndexDB;
    }

private:
    void PublishDiagnostics(const PublishDiagnosticsParams &params);

    void OnInitialize(const InitializeParams &params, nlohmann::json id);

    void OnInitialized();

    void OnShutdown(nlohmann::json id);

    void OnDocumentDidOpen(const DidOpenTextDocumentParams &params);

    void OnDocumentDidChange(const DidChangeTextDocumentParams &params);

    void OnDocumentDidClose(const DidCloseTextDocumentParams &params);

    void OnTrackCompletion(const TrackCompletionParams &params);

    void OnDocumentHighlight(const TextDocumentPositionParams &params, nlohmann::json documentHighlightId);

    void OnReference(const TextDocumentPositionParams &params, nlohmann::json id);

    void OnFileReference(const DocumentLinkParams &params, nlohmann::json id);

    void OnGoToDefinition(const TextDocumentPositionParams &params, nlohmann::json id);

    void OnCrossLanguageGoToDefinition(const CrossLanguageJumpParams &params, nlohmann::json id);

    void OnSignatureHelp(const SignatureHelpParams &params, nlohmann::json id);

    void OnHover(const TextDocumentPositionParams &params, nlohmann::json onHoverId);

    void OnDocumentLink(const DocumentLinkParams &params, nlohmann::json id);

    void OnCompletion(const CompletionParams &params, nlohmann::json id);

    void OnPrepareRename(const TextDocumentPositionParams &params, nlohmann::json id);

    void OnRename(const RenameParams &params, nlohmann::json id);

    void OnBreakpoints(const TextDocumentParams &params, nlohmann::json id);

    void OnCodeLens(const TextDocumentParams &params, nlohmann::json id);

    void OnWorkspaceSymbol(const WorkspaceSymbolParams &params, nlohmann::json id);

    void Notify(const std::string &method, const ValueOrError &params);

    bool AllowCompletion(const CompletionParams &params);

    void OnSemanticTokens(const SemanticTokensParams &params, nlohmann::json id);

    bool PerformCompiler(const InitializeParams &params);

    void OnPrepareTypeHierarchy(const TextDocumentPositionParams &params, nlohmann::json typeHierarchyId);

    void OnSupertypes(const TypeHierarchyItem &params, nlohmann::json id);

    void OnSubtypes(const TypeHierarchyItem &params, nlohmann::json id);

    void OnPrepareCallHierarchy(const TextDocumentPositionParams &params, nlohmann::json callHierarchyId);

    void OnIncomingCalls(const CallHierarchyItem &params, nlohmann::json id);

    void OnOutgoingCalls(const CallHierarchyItem &params, nlohmann::json id);

    void OnDidChangeWatchedFiles(const DidChangeWatchedFilesParam &params);

    bool CheckFileInCangjieProject(const std::string &filePath, bool ignoreMacro = true) const;

    bool CheckIsDirectory(const std::string &dirPath, bool isDelete = false) const;

    void OnDocumentSymbol(const DocumentSymbolParams &params, nlohmann::json id);

    void OnOverrideMethods(const OverrideMethodsParams &params, nlohmann::json id);

    void AutoImportQuickFixPrepare(DiagnosticToken &diagnostic, ArkAST *arkAst);

    void AddAutoImportQuickFix(DiagnosticToken &diagnostic, const std::string& identifier, Ptr<const File> file,
        Cangjie::ImportManager *importManager);

    void HandleExternalImportSym(std::vector<CodeAction> &actions, const std::string &pkg,
        const lsp::Symbol &sym, Range textEditRange, const std::string &uri);

    void OnCodeAction(const CodeActionParams &,  nlohmann::json id);

    void OnCommand(const ExecuteCommandParams &,  nlohmann::json id);

    void OnCommandApplyTweak(const TweakArgs &,  nlohmann::json id);

    void AddAllImportCodeAction(std::vector<DiagnosticToken> &diagnostics, const std::string& uri);

    bool NeedCollect2AllImport(const DiagnosticToken &diagnostic);

    void CollectCA2AllImport(std::vector<CodeAction> &allImports, const std::vector<CodeAction> &codeActions);

    void ReplyError(const nlohmann::json &id) const
    {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        nlohmann::json result = nullptr;
        transp.Reply(std::move(id), ValueOrError(ValueOrErrorCheck::VALUE, result));
    }

    std::mutex fixItsMutex {};

    std::unordered_map<std::string, std::set<DiagnosticToken, DiagnosticCompare>> fixItsMap {};

    Transport &transp;

    class MessageHandler; // defined in ArkLanguageServer.cpp

    std::unique_ptr<MessageHandler> MsgHandler;

    DocCache DocMgr {};

    std::unique_ptr<ArkServer> Server;

    ClientCapabilities clientCapabilities;

    Environment envs;

    TextDocumentSyncKind syncKind {TextDocumentSyncKind::SK_NONE};

    lsp::IndexDatabase *arkIndexDB = nullptr;
};
} // namespace ark

#endif // LSPSERVER_ARKLANGUAGESERVER_H
