// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <iostream>
#include <map>
#include <set>
#include <mutex>
#include <utility>
#include <cctype>

#include "cangjie/Frontend/CompilerInstance.h"
#include "capabilities/semanticHighlight/SemanticTokensAdaptor.h"
#include "capabilities/shutdown/Shutdown.h"
#include "common/Utils.h"
#include "ArkLanguageServer.h"

namespace ark {
using namespace Cangjie;
using namespace Cangjie::FileUtil;
using namespace CONSTANTS;

namespace {
using TweakReply = std::function<void(const ValueOrError &)>;

void ReplyTweakNoEdit(const TweakReply &reply)
{
    ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
    reply(value);
}

Range BuildTweakSelectionRange(unsigned int fileId, const TweakArgs &args)
{
    Range range;
    range.start = Cangjie::Position{
        fileId,
        args.selection.start.line,
        args.selection.start.column
    };
    range.end = Cangjie::Position{
        fileId,
        args.selection.end.line,
        args.selection.end.column
    };
    return range;
}

ValueOrError BuildTweakWorkspaceEditValue(Tweak::Effect effect)
{
    ApplyWorkspaceEditParams editParams;
    editParams.edit.changes = std::move(effect.applyEdits);
    editParams.edit.documentChanges = std::move(effect.documentChanges);
    nlohmann::json res;
    ToJSON(editParams, res);

    return ValueOrError(ValueOrErrorCheck::VALUE, res);
}
} // namespace

// MessageHandler dispatches incoming LSP messages.
// It handles cross-cutting concerns:
//  - serialize/deserialize protocol objects to JSON
//  - logging of inbound messages
//  - cancellation handling
//  - basic call tracing
// MessageHandler ensures that initialize() is called before any other handler.
class ArkLanguageServer::MessageHandler : public Transport::MessageHandler {
public:
    explicit MessageHandler(ArkLanguageServer &server) : server(server) {}
    ~MessageHandler() override {}

    LSPRet OnNotify(std::string method, nlohmann::json params) override
    {
        std::stringstream log;
        Logger& logger = Logger::Instance();
        if (!getInitialize || (!isInitialized && method != "initialized")) {
            CleanAndLog(log, "Initialization is needed before Notification:" + method);
            logger.LogMessage(MessageType::MSG_WARNING, log.str());
            return LSPRet::SUCCESS;
        }
        if (ShutdownRequested()) {
            logger.LogMessage(MessageType::MSG_WARNING, "server already shutdown");
            return LSPRet::SUCCESS;
        }
        auto iter = notifications.find(method);
        if (iter != notifications.end()) {
            iter->second(std::move(params));
            return LSPRet::SUCCESS;
        }
        if (method == "textDocument/didSave") {
            std::string fileUri = params["textDocument"].value("uri", "");
            if (fileUri.empty()) {
                logger.LogMessage(MessageType::MSG_WARNING, "The file to be saved does not exist");
                return LSPRet::SUCCESS;
            }
            std::string path = FileStore::NormalizePath(URI::Resolve(fileUri));
            logger.LogMessage(MessageType::MSG_LOG, "The file " + GetFileName(path) + " has been saved successfully");
            return LSPRet::SUCCESS;
        }
        CleanAndLog(log, "unhandled notification:" + method);
        logger.LogMessage(MessageType::MSG_WARNING, log.str());
        return LSPRet::SUCCESS;
    }

    LSPRet OnCall(std::string method, nlohmann::json params, nlohmann::json id) override
    {
        std::stringstream log;
        Logger& logger = Logger::Instance();
        if (!isInitialized && method != "initialize") {
            CleanAndLog(log, "initialize is necessary before call:" + method + ", and reply error to client.");
            logger.LogMessage(MessageType::MSG_WARNING, log.str());
            std::lock_guard<std::mutex> lock(server.transp.transpWriter);
            server.transp.Reply(std::move(id),
                                std::move(ValueOrError(ValueOrErrorCheck::ERR,
                                   MessageErrorDetail("server not initialized",
                                                                        ErrorCode::SERVER_NOT_INITIALIZED))));
            return LSPRet::SUCCESS;
        }
        if (ShutdownRequested()) {
            std::lock_guard<std::mutex> lock(server.transp.transpWriter);
            server.transp.Reply(id, ValueOrError(ValueOrErrorCheck::ERR,
                                        MessageErrorDetail("server already shutdown",
                                                                  ErrorCode::INVALID_REQUEST)));
            return LSPRet::SUCCESS;
        }

        auto iter = calls.find(method);
        if (iter != calls.end()) {
            iter->second(std::move(params), id);
            return LSPRet::SUCCESS;
        }
        std::lock_guard<std::mutex> lock(server.transp.transpWriter);
        CleanAndLog(log, "method not found, and reply error to client, method name:" + method);
        logger.LogMessage(MessageType::MSG_WARNING, log.str());
        server.transp.Reply(std::move(id),
                            std::move(ValueOrError(ValueOrErrorCheck::ERR,
                                               MessageErrorDetail("method not found",
                                                                    ErrorCode::METHOD_NOT_FOUND))));
        return LSPRet::SUCCESS;
    }

    // ReplyCallbacks used in onReply is not written.
    // This function is not used currently and has not been debugged. lxw
    LSPRet OnReply(nlohmann::json id, ValueOrError result) override
    {
        std::stringstream log;
        Logger& logger = Logger::Instance();
        Callback<nlohmann::json> replyHandler = nullptr;
#ifndef NO_EXCEPTIONS
        try {
#endif
            auto intID = id.get<int>();
            replyHandler = TakeReplyCallback(intID);
            if (!replyHandler) {
                // No callback being found, use a default log callback.
                CleanAndLog(log, "received a reply with ID:" + std::to_string(id.get<int>()) +
                                     ", but there was no such call");
                logger.LogMessage(MessageType::MSG_WARNING, log.str());
                return LSPRet::SUCCESS;
            }
            // LCOV_EXCL_START
            // Log and run the reply handler.
            if (result.type == ValueOrErrorCheck::VALUE) {
                CleanAndLog(log, "<-- reply:" + std::to_string(id.get<int>()) + ", success");
                logger.LogMessage(MessageType::MSG_INFO, log.str());
                replyHandler(std::move(result.jsonValue));
            } else {
                CleanAndLog(log, "<-- reply:" + std::to_string(id.get<int>()) + ", error:" +
                    std::to_string(static_cast<int>(result.errorInfo.code)) +
                            "," + result.errorInfo.message);
                logger.LogMessage(MessageType::MSG_WARNING, log.str());
            }
            // LCOV_EXCL_STOP
            return LSPRet::SUCCESS;
#ifndef NO_EXCEPTIONS
        } catch (const std::exception &e) {
            CleanAndLog(log, "Invalid reply: " + std::string(e.what()));
            logger.LogMessage(MessageType::MSG_WARNING, log.str());
            return LSPRet::ABNORMAL_EXIT;
        }
#endif
    }

    // Bind an LSP method name to a call.
    template<typename Param>
    void Bind(const char *method, void (ArkLanguageServer::*handler)(const Param &, nlohmann::json))
    {
        calls[method] = [method, handler, this](const nlohmann::json &rawParams, const nlohmann::json &id) {
            Param param;
            // LCOV_EXCL_START
            if (!FromJSON(rawParams, param)) {
                std::stringstream log;
                log << "Failed to decode request or request not meet requirement, method:" << method;
                Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
                return;
            }
            (server.*handler)(param, id);
            // LCOV_EXCL_STOP
        };
    }

    // Bind an LSP method name to a call without param.
    void Bind(const char *method, void (ArkLanguageServer::*handler)(nlohmann::json))
    {
        calls[method] = [handler, this](const nlohmann::json &, const nlohmann::json &id) {
            (server.*handler)(id);
        };
    }

    // Bind an LSP method name to a notification.
    template<typename Param>
    void Bind(const char *method, void (ArkLanguageServer::*handler)(const Param &))
    {
        notifications[method] = [method, handler, this](const nlohmann::json &rawParams) {
            Param param;
            if (!FromJSON(rawParams, param)) {
                std::stringstream log;
                log << "Failed to decode request or request not meet requirement, method:" << method;
                Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
                return;
            }
            (server.*handler)(param);
        };
    }

    // Bind an LSP method name to a notification without param.
    void Bind(const char *method, void (ArkLanguageServer::*handler)())
    {
        notifications[method] = [handler, this](const nlohmann::json &) {
            (server.*handler)();
        };
    }

    bool WhetherGetInitialized() const
    {
        return this->isInitialized;
    }

    void SetInitialize(bool getInit)
    {
        this->getInitialize = getInit;
    }
    void SetInitialized(bool beInitialized)
    {
        this->isInitialized = beInitialized;
    }

    Callback<nlohmann::json> TakeReplyCallback(int intID)
    {
        if (intID == 0) {
            return nullptr;
        }
        std::lock_guard<std::mutex> mutex(callMutex);
        // Find a corresponding callback for the request ID;
        for (size_t index = 0; index < replyCallbacks.size(); ++index) {
            if (replyCallbacks[index].first == intID) {
                auto handler = std::move(replyCallbacks[index].second);
                // remove the entry
                (void)replyCallbacks.erase(replyCallbacks.begin() + static_cast<long long>(index));
                return handler;
            }
        }
        return nullptr;
    }

private:

    std::map<std::string, std::function<void(const nlohmann::json &)>> notifications;
    std::map<std::string, std::function<void(const nlohmann::json &, const nlohmann::json &)>> calls;

    mutable std::mutex callMutex {};
    // RequestID and ReplyHandler
    std::deque<std::pair<int, Callback<nlohmann::json>>> replyCallbacks {};

    bool getInitialize = false;
    bool isInitialized = false;
    ArkLanguageServer &server;
};

ArkLanguageServer::ArkLanguageServer(Transport &transport, Environment environmentVars, lsp::IndexDatabase *db)
    : transp(transport), MsgHandler(std::make_unique<MessageHandler>(*this)), Server(std::make_unique<ArkServer>(this)),
      envs(environmentVars), arkIndexDB(db)
{
    CompilerCangjieProject::InitInstance(this, arkIndexDB);

    MsgHandler->Bind("initialize", &ArkLanguageServer::OnInitialize);
    MsgHandler->Bind("initialized", &ArkLanguageServer::OnInitialized);
    MsgHandler->Bind("shutdown", &ArkLanguageServer::OnShutdown);
    MsgHandler->Bind("textDocument/semanticTokens", &ArkLanguageServer::OnSemanticTokens);
    // for IDEA
    MsgHandler->Bind("textDocument/semanticTokens/full", &ArkLanguageServer::OnSemanticTokens);
    MsgHandler->Bind("textDocument/didOpen", &ArkLanguageServer::OnDocumentDidOpen);
    MsgHandler->Bind("textDocument/didClose", &ArkLanguageServer::OnDocumentDidClose);
    MsgHandler->Bind("textDocument/didChange", &ArkLanguageServer::OnDocumentDidChange);
    MsgHandler->Bind("textDocument/trackCompletion", &ArkLanguageServer::OnTrackCompletion);
    MsgHandler->Bind("textDocument/documentLink", &ArkLanguageServer::OnDocumentLink);
    MsgHandler->Bind("textDocument/documentHighlight", &ArkLanguageServer::OnDocumentHighlight);
    MsgHandler->Bind("textDocument/prepareTypeHierarchy", &ArkLanguageServer::OnPrepareTypeHierarchy);
    MsgHandler->Bind("textDocument/prepareCallHierarchy", &ArkLanguageServer::OnPrepareCallHierarchy);
    MsgHandler->Bind("callHierarchy/incomingCalls", &ArkLanguageServer::OnIncomingCalls);
    MsgHandler->Bind("callHierarchy/outgoingCalls", &ArkLanguageServer::OnOutgoingCalls);
    MsgHandler->Bind("typeHierarchy/supertypes", &ArkLanguageServer::OnSupertypes);
    MsgHandler->Bind("typeHierarchy/subtypes", &ArkLanguageServer::OnSubtypes);
    MsgHandler->Bind("textDocument/definition", &ArkLanguageServer::OnGoToDefinition);
    MsgHandler->Bind("textDocument/references", &ArkLanguageServer::OnReference);
    MsgHandler->Bind("textDocument/signatureHelp", &ArkLanguageServer::OnSignatureHelp);
    MsgHandler->Bind("textDocument/hover", &ArkLanguageServer::OnHover);
    MsgHandler->Bind("textDocument/completion", &ArkLanguageServer::OnCompletion);
    MsgHandler->Bind("textDocument/prepareRename", &ArkLanguageServer::OnPrepareRename);
    MsgHandler->Bind("textDocument/rename", &ArkLanguageServer::OnRename);
    MsgHandler->Bind("workspace/symbol", &ArkLanguageServer::OnWorkspaceSymbol);
    MsgHandler->Bind("workspace/didChangeWatchedFiles", &ArkLanguageServer::OnDidChangeWatchedFiles);
    MsgHandler->Bind("textDocument/documentSymbol", &ArkLanguageServer::OnDocumentSymbol);
    MsgHandler->Bind("textDocument/breakpoints", &ArkLanguageServer::OnBreakpoints);
    MsgHandler->Bind("textDocument/crossLanguageDefinition", &ArkLanguageServer::OnCrossLanguageGoToDefinition);
    MsgHandler->Bind("codeGenerator/overrideMethods", &ArkLanguageServer::OnOverrideMethods);
    MsgHandler->Bind("textDocument/codeAction", &ArkLanguageServer::OnCodeAction);
    MsgHandler->Bind("workspace/executeCommand", &ArkLanguageServer::OnCommand);
    MsgHandler->Bind("textDocument/findFileReferences", &ArkLanguageServer::OnFileReference);
    MsgHandler->Bind("textDocument/exportsName", &ArkLanguageServer::OnExportsName);
    MsgHandler->Bind("textDocument/crossLanguageRegister", &ArkLanguageServer::OnCrossLanguageRegister);
    MsgHandler->Bind("textDocument/fileRefactor", &ArkLanguageServer::OnFileRefactor);
    if (!MessageHeaderEndOfLine::GetIsDeveco()) {
        MsgHandler->Bind("textDocument/codeLens", &ArkLanguageServer::OnCodeLens);
    }
    syncKind = TextDocumentSyncKind::SK_INCREMENTAL;
}

ArkLanguageServer::~ArkLanguageServer() {}

std::string ArkLanguageServer::CreateProgressToken()
{
    static std::atomic<uint64_t> tokenCounter{0};
    return "progress_" + std::to_string(tokenCounter.fetch_add(1));
}

void ArkLanguageServer::SendWorkDoneProgressCreate(const WorkDoneProgressCreateParams &params)
{
    nlohmann::json jsonParams;
    if (!ToJSON(params, jsonParams)) {
        return;
    }
    ValueOrError result(ValueOrErrorCheck::VALUE, jsonParams);
    Notify("window/workDoneProgress/create", result);
}

void ArkLanguageServer::SendWorkDoneProgressBegin(const std::string &token, const WorkDoneProgressBegin &begin)
{
    WorkDoneProgressParams params;
    params.token = token;
    params.workDoneProgressBegin = begin;

    nlohmann::json jsonParams;
    if (!ToJSON(params, jsonParams)) {
        return;
    }
    ValueOrError result(ValueOrErrorCheck::VALUE, jsonParams);
    Notify("$/progress", result);
}

void ArkLanguageServer::SendWorkDoneProgressReport(const std::string &token, const WorkDoneProgressReport &report)
{
    WorkDoneProgressParams params;
    params.token = token;
    params.workDoneProgressReport = report;

    nlohmann::json jsonParams;
    if (!ToJSON(params, jsonParams)) {
        return;
    }
    ValueOrError result(ValueOrErrorCheck::VALUE, jsonParams);
    Notify("$/progress", result);
}

void ArkLanguageServer::SendWorkDoneProgressEnd(const std::string &token, const WorkDoneProgressEnd &end)
{
    WorkDoneProgressParams params;
    params.token = token;
    params.workDoneProgressEnd = end;

    nlohmann::json jsonParams;
    if (!ToJSON(params, jsonParams)) {
        return;
    }
    ValueOrError result(ValueOrErrorCheck::VALUE, jsonParams);
    Notify("$/progress", result);
}

LSPRet ArkLanguageServer::Run() const
{
    // Run the Language Server loop.
    return transp.Loop(*MsgHandler);
}

nlohmann::json GetServerInfo()
{
    nlohmann::json serverInfo;
    serverInfo["name"] = "Cangjie language server";
    serverInfo["version"] = "1.0";
    return serverInfo;
}

nlohmann::json GetServerCapabilities(int syncKind)
{
    nlohmann::json serverCapabilities;
    serverCapabilities["textDocumentSync"] = syncKind;
    serverCapabilities["documentHighlightProvider"] = true;
    serverCapabilities["semaHighlightProvider"] = true;
    serverCapabilities["referencesProvider"] = true;
    serverCapabilities["definitionProvider"] = true;
    serverCapabilities["hoverProvider"] = true;
    serverCapabilities["workspaceSymbolProvider"] = true;
    serverCapabilities["documentSymbolProvider"] = true;
    serverCapabilities["renameProvider"]["prepareProvider"] = true;
    serverCapabilities["typeHierarchyProvider"] = true;
    serverCapabilities["callHierarchyProvider"] = true;
    serverCapabilities["documentLinkProvider"]["resolveProvider"] = true;
    serverCapabilities["completionProvider"]["resolveProvider"] = false;
    serverCapabilities["breakpointsProvider"] = true;
    if (!MessageHeaderEndOfLine::GetIsDeveco()) {
        serverCapabilities["codeLensProvider"]["resolveProvider"] = true;
    }
    serverCapabilities["crossLanguageDefinition"] = true;
    serverCapabilities["findFileReferences"] = true;
    serverCapabilities["exportsName"] = true;
    serverCapabilities["crossLanguageRegister"] = true;
    std::set<std::string> triggerCharacters = {".", "`"};
    for (const std::string& item : triggerCharacters) {
        (void)serverCapabilities["completionProvider"]["triggerCharacters"].push_back(item);
    }
    std::set<std::string> signatureHelpTriggerCharacters = {"(", ","};
    for (const std::string &item : signatureHelpTriggerCharacters) {
        (void)serverCapabilities["signatureHelpProvider"]["triggerCharacters"].push_back(item);
    }
    // tokenTypes externly defined in SemanticTokenAdaptor
    for (const std::string &tokenItem: SemanticTokensAdaptor::TOKEN_TYPES) {
        (void)serverCapabilities["semanticTokensProvider"]["legend"]["tokenTypes"].push_back(tokenItem);
    }
    for (const std::string &modifierItem: SemanticTokensAdaptor::TOKEN_MODIFIERS) {
        (void) serverCapabilities["semanticTokensProvider"]["legend"]["tokenModifiers"].push_back(modifierItem);
    }
    // adapt for vscode clangd
    serverCapabilities["semanticTokensProvider"]["range"] = false;
    serverCapabilities["semanticTokensProvider"]["full"]["delta"] = true;

    if (MessageHeaderEndOfLine::GetIsDeveco()) {
        serverCapabilities["codeActionProvider"] = true;
        serverCapabilities["executeCommandProvider"]["commands"] = { Command::APPLY_EDIT_COMMAND };
    }
    serverCapabilities["findFileRefactor"] = true;

    return serverCapabilities;
}

ValueOrError ReplyInitialize(int syncKind)
{
    nlohmann::json value;

    nlohmann::json serverInfo = GetServerInfo();
    value["serverInfo"] = serverInfo;

    nlohmann::json serverCapabilities = GetServerCapabilities(static_cast<int>(syncKind));
    value["capabilities"] = serverCapabilities;

    ValueOrError result(ValueOrErrorCheck::VALUE, value);
    return result;
}

void ArkLanguageServer::OnInitialize(const InitializeParams &params, nlohmann::json id)
{
    if (MsgHandler->WhetherGetInitialized()) {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(id, ValueOrError(ValueOrErrorCheck::ERR,
                             MessageErrorDetail("server already initialized", ErrorCode::INVALID_REQUEST)));
        return;
    }

    bool success = PerformCompiler(params);
    if (!success) {
        transp.Reply(std::move(id), std::move(ValueOrError(ValueOrErrorCheck::ERR,
                                        MessageErrorDetail("initialize fail",
                                                                            ErrorCode::SERVER_NOT_INITIALIZED))));
        return;
    }
    MsgHandler->SetInitialize(true);
    clientCapabilities = params.capabilities;
    ValueOrError result = ReplyInitialize(static_cast<int>(syncKind));
    std::lock_guard<std::mutex> lock(transp.transpWriter);
    transp.Reply(std::move(id), result);
}

bool ArkLanguageServer::PerformCompiler(const InitializeParams &params)
{
    auto ret = CompilerCangjieProject::GetInstance()->Compiler(params.rootUri.file, params.initializationOptions, envs);
    return ret;
}

void ArkLanguageServer::OnInitialized()
{
    MsgHandler->SetInitialized(true);
}

std::string ArkLanguageServer::GetContentsByFile(const std::string &file)
{
    DocCache::Doc doc = DocMgr.GetDoc(file);
    return doc.contents;
}

int64_t ArkLanguageServer::GetVersionByFile(const std::string &file)
{
    DocCache::Doc doc = DocMgr.GetDoc(file);
    return doc.version;
}

bool ArkLanguageServer::NeedReParser(const std::string &file)
{
    DocCache::Doc doc = DocMgr.GetDoc(file);
    return doc.needReParser;
}

void ArkLanguageServer::UpdateDocNeedReparse(const std::string &file, int64_t version, bool needReParser)
{
    DocMgr.UpdateDocNeedReparse(file, version, needReParser);
}

void ArkLanguageServer::AddDocWhenInitCompile(const std::string &file)
{
    DocMgr.AddDocWhenInitCompile(file);
}

void ArkLanguageServer::OnShutdown(nlohmann::json id)
{
    RequestShutdown();
    nlohmann::json value;
    ValueOrError result(ValueOrErrorCheck::VALUE, value);
    std::lock_guard<std::mutex> lock(transp.transpWriter);
    transp.Reply(std::move(id), result);
}

void ArkLanguageServer::OnDocumentDidOpen(const DidOpenTextDocumentParams &params)
{
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!FileUtil::FileExist(file) || !CheckFileInCangjieProject(file)) {
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    const std::string &contents = params.textDocument.text;
    bool reBuild = false;
    // file not in pkgInfo or content change need reBuild
    if (!doc.isInitCompiled || (doc.version != -1 && doc.contents != contents)) {
        reBuild = true;
    }
    if (IsInCjlibDir(file)) {
        reBuild = false;
    }
    if (doc.version == -1 && MessageHeaderEndOfLine::GetIsDeveco()) {
        if (doc.isInitCompiled) {
            auto buffer = CompilerCangjieProject::GetInstance()->GetContentByFile(file);
            std::regex reg("\r\n");
            std::string replaceStr = "\n";
            std::string bufferResult = std::regex_replace(buffer, reg, replaceStr);
            std::string contentsResult = std::regex_replace(contents, reg, replaceStr);
            if (!buffer.empty() && bufferResult != contentsResult) {
                reBuild = true;
            }
        }
    }
    if (reBuild) {
        auto pkgName = CompilerCangjieProject::GetInstance()->GetFullPkgName(file);
        CompilerCangjieProject::GetInstance()->
            UpdateFileStatusInCI(pkgName, file,CompilerInstance::SrcCodeChangeState::CHANGED);
    }
    int64_t version = DocMgr.AddDoc(file, params.textDocument.version, contents);
    Server->AddDoc(file, contents, version, ark::NeedDiagnostics::YES, reBuild);
}

// close file do not clear file contents cache, leave watch file delete to do it
void ArkLanguageServer::OnDocumentDidClose(const DidCloseTextDocumentParams &params)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::onDocumentDidClose in");

    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        return;
    }
    PublishDiagnosticsParams notification;
    notification.uri.file = params.textDocument.uri.file;
    PublishDiagnostics(notification);
    if (Options::GetInstance().GetLSPFlag("enableParallel").has_value()) {
        if (!Options::GetInstance().GetLSPFlag("enableParallel").value()) {
            return;
        }
    }
    if (!MessageHeaderEndOfLine::GetIsDeveco()) {
        CompilerCangjieProject::GetInstance()->UpdateOnDisk(file);
    }
}

void ArkLanguageServer::OnDocumentDidChange(const DidChangeTextDocumentParams &params)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnDocumentDidChange in");
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        return;
    }
    if (!DocMgr.UpdateDoc(file, params.textDocument.version, true, params.contentChanges)) {
        logger.LogMessage(MessageType::MSG_WARNING, "file:" + file + " not exist");
        return;
    }
    if (!params.contentChanges.empty()) {
        CompilerCangjieProject::GetInstance()->UpdateBuffCache(file, true);
    }
}

void ArkLanguageServer::OnTrackCompletion(const TrackCompletionParams &params)
{
    Trace::Log("OnTrackCompletion in");
    CompilerCangjieProject::GetInstance()->UpdateUsageFrequency(params.label);
}

void ArkLanguageServer::OnDocumentHighlight(const TextDocumentPositionParams &params,
    nlohmann::json documentHighlightId)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnDocumentHighlight in.");

    if (!clientCapabilities.textDocumentClientCapabilities.documentHighlightClientCapabilities) {
        Logger::Instance().LogMessage(MessageType::MSG_WARNING,
            "client declare not to support documenthighlight in initialize");
        ReplyError(documentHighlightId);
        return;
    }

    // check didopen was received before documentHighlight
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(documentHighlightId);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnDocumentHighlight, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(documentHighlightId);
        return;
    }

    auto reply = [documentHighlightId, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(documentHighlightId), std::move(result));
    };
    Server->FindDocumentHighlights(file, params, std::move(reply));
}

void ArkLanguageServer::OnPrepareTypeHierarchy(const TextDocumentPositionParams &params, nlohmann::json typeHierarchyId)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnPrepareTypeHierarchy in.");
    // vscode currently does not include this Capability in message
    if (!clientCapabilities.textDocumentClientCapabilities.typeHierarchyCapabilities) {
        Logger::Instance().LogMessage(MessageType::MSG_WARNING,
                                      "client declare not to support PrepareTypeHierarchy in initialize");
    }

    // check didopen was received before documentHighlight
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(typeHierarchyId);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnPrepareTypeHierarchy, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(typeHierarchyId);
        return;
    }

    auto reply = [typeHierarchyId, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(typeHierarchyId), std::move(result));
    };
    Server->FindTypeHierarchys(file, params, std::move(reply));
}

void ArkLanguageServer::OnSupertypes(const TypeHierarchyItem &params, nlohmann::json id)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnSupertypes in.");
    std::string file = FileStore::NormalizePath(URI::Resolve(params.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindSuperTypes(file, params, std::move(reply));
}

void ArkLanguageServer::OnSubtypes(const TypeHierarchyItem &params, nlohmann::json id)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnSubtypes in.");
    std::string file = FileStore::NormalizePath(URI::Resolve(params.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindSubTypes(file, params, std::move(reply));
}

void ArkLanguageServer::OnPrepareCallHierarchy(const TextDocumentPositionParams &params, nlohmann::json callHierarchyId)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnPrepareCallHierarchy in.");
    // vscode currently does not include this Capability in message
    if (!clientCapabilities.textDocumentClientCapabilities.typeHierarchyCapabilities) {
        Logger::Instance().LogMessage(MessageType::MSG_WARNING,
                                      "client declare not to support PrepareCallHierarchy in initialize");
    }

    // check didopen was received before documentHighlight
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(callHierarchyId);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnPrepareCallHierarchy, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(callHierarchyId);
        return;
    }

    auto reply = [callHierarchyId, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(callHierarchyId), std::move(result));
    };
    Server->FindCallHierarchys(file, params, std::move(reply));
}

void ArkLanguageServer::OnIncomingCalls(const CallHierarchyItem &params, nlohmann::json id)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnIncomingCalls in.");
    std::string file = FileStore::NormalizePath(URI::Resolve(params.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindOnIncomingCalls(file, params, std::move(reply));
}

void ArkLanguageServer::OnOutgoingCalls(const CallHierarchyItem &params, nlohmann::json id)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnOutgoingCalls in.");
    std::string file = FileStore::NormalizePath(URI::Resolve(params.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindOnOutgoingCalls(file, params, std::move(reply));
}

void ArkLanguageServer::OnDocumentLink(const DocumentLinkParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnDocumentLink in.");

    if (!clientCapabilities.textDocumentClientCapabilities.documentLinkClientCapabilities) {
        Logger::Instance().LogMessage(MessageType::MSG_WARNING,
                                      "client declare not to support documentLink in initialize");
        ReplyError(id);
        return;
    }

    // check didopen was received before documentLink
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file, false)) {
        ReplyError(id);
        return;
    }
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindDocumentLink(file, std::move(reply));
}
// LCOV_EXCL_START
void ArkLanguageServer::WrapClientWatchedFiles(std::vector<FileWatchedEvent> &changes,
                                               const DidChangeWatchedFilesParam &params) const
{
    for (auto &event : params.changes) {
        std::set<std::string> fileNameSet;
        std::string file = FileStore::NormalizePath(URI::Resolve(event.textDocument.uri.file));
        // 3 indicate ".cj" string length
        if (file.rfind(".cj") == file.size() - 3 &&
        fileNameSet.find(event.textDocument.uri.file) == fileNameSet.end()) {
            (void)fileNameSet.insert(event.textDocument.uri.file);
            changes.push_back({event.textDocument, event.type}); // if this is a cj file
            continue;
        }
        std::string filePath = file + "/dump.file";

        std::vector<std::string> fileVec;
        if (event.type == FileChangeType::DELETED && CheckIsDirectory(file, true) &&
            CheckFileInCangjieProject(filePath)) {
            auto res =
                CompilerCangjieProject::GetInstance()->GetAllFilesUnderPathRecursive(file, true); // multi folder
            std::for_each(res.begin(), res.end(), [&file, &fileVec] (auto& fp) {
                if (fp.rfind(file, 0) == 0) {
                    fileVec.push_back(fp.substr(file.length() + 1));
                }
            });
        } else if (event.type == FileChangeType::CREATED && CheckIsDirectory(file) &&
                CheckFileInCangjieProject(filePath)) {
            fileVec = GetAllFilesUnderCurrentPath(file, CANGJIE_FILE_EXTENSION(), false);
        }
        for (const auto& item : fileVec) {
            TextDocumentIdentifier textDocument;
            textDocument.uri.file = event.textDocument.uri.file + "/" + item; // format: file:///d%3A/Code/LSP
            if (fileNameSet.find(textDocument.uri.file) == fileNameSet.end()) {
                (void)fileNameSet.insert(textDocument.uri.file);
                changes.push_back({textDocument, event.type});
            }
        }
    }
}

void ArkLanguageServer::OnDidChangeWatchedFiles(const DidChangeWatchedFilesParam &params)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnDidChangeWatchedFiles in.");

    std::vector<FileWatchedEvent> changes;
    WrapClientWatchedFiles(changes, params);
    for (auto &event : changes) {
        std::string file = FileStore::NormalizePath(URI::Resolve(event.textDocument.uri.file));
        if (!CheckFileInCangjieProject(file)) {
            logger.LogMessage(MessageType::MSG_INFO, "Not in src directory");
            continue;
        }
        Server->ChangeWatchedFiles(file, event.type, &DocMgr);
    }
}
// LCOV_EXCL_STOP
bool ArkLanguageServer::CheckFileInCangjieProject(const std::string &filePath, bool ignoreMacro) const
{
    if (filePath.empty() || (ignoreMacro && Cangjie::FileUtil::HasExtension(filePath,
        CANGJIE_MACRO_FILE_EXTENSION()))) {
        return false;
    }
    return CompilerCangjieProject::GetInstance()->GetCangjieFileKind(filePath).first != CangjieFileKind::MISSING;
}
// LCOV_EXCL_START
bool ArkLanguageServer::CheckPkgInCangjieProject(const std::string &pkgPath) const
{
    return CompilerCangjieProject::GetInstance()->GetCangjieFileKind(pkgPath, true).first != CangjieFileKind::MISSING;
}

void ArkLanguageServer::RemoveDocByFile(const std::string &file)
{
    DocMgr.RemoveDoc(file);
}
// LCOV_EXCL_STOP
void ArkLanguageServer::OnHover(const TextDocumentPositionParams &params, nlohmann::json onHoverId)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnHover in.");

    if (!clientCapabilities.textDocumentClientCapabilities.hoverClientCapabilities) {
        Logger::Instance().LogMessage(MessageType::MSG_WARNING,
                                      "client declare not to support hover in initialize");
        ReplyError(onHoverId);
        return;
    }

    // check didopen was received before hover
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(onHoverId);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnHover, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(onHoverId);
        return;
    }

    auto reply = [onHoverId, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(onHoverId), std::move(result));
    };
    Server->FindHover(file, params, std::move(reply));
}

void ArkLanguageServer::OnReference(const TextDocumentPositionParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnReference in");

    // check didopen was received before onReference
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnReference, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindReferences(file, params, std::move(reply));
}

void ArkLanguageServer::OnFileReference(const DocumentLinkParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnFileReference in");

    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindFileReferences(file, std::move(reply));
}
// LCOV_EXCL_START
void ArkLanguageServer::OnCrossLanguageRegister(const CrossLanguageJumpParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnCrossLanguageRegister in");
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->LocateRegisterCrossSymbolAt(params, std::move(reply));
}

void ArkLanguageServer::OnExportsName(const ExportsNameParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnExportsName in");

    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnExportsName, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(id);
        return;
    }
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->GetExportsName(file, params, std::move(reply));
}

void ArkLanguageServer::OnFileRefactor(const FileRefactorReqParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnFileRefactor in");

    std::string file = FileStore::NormalizePath(URI::Resolve(params.file.uri.file));
    if (FileUtil::GetFileExtension(file) != CONSTANTS::CANGJIE_FILE_EXTENSION()) {
        ReplyError(id);
        return;
    }
    std::string selectedElement = FileStore::NormalizePath(URI::Resolve(params.selectedElement.uri.file));
    bool fileInCjProject = FileUtil::IsDir(selectedElement) ? CheckPkgInCangjieProject(selectedElement)
                                                            : CheckFileInCangjieProject(selectedElement);
    std::string target = FileStore::NormalizePath(URI::Resolve(params.targetPath.uri.file));
    target = FileUtil::IsDir(target) ? target :FileUtil::GetDirPath(target);
    bool targetInCjProject = CheckPkgInCangjieProject(target);
    if (!fileInCjProject || !targetInCjProject) {
        ReplyError(id);
        return;
    }

    auto reply = [id, this](const ValueOrError &result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), result);
    };

    bool isTest = false;
    Server->ApplyFileRefactor(file, selectedElement, target, isTest, std::move(reply));
}
// LCOV_EXCL_STOP

void ArkLanguageServer::OnGoToDefinition(const TextDocumentPositionParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnGoToDefinition in");

    // check didopen was received before onGoToDefinition
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnGoToDefinition, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->LocateSymbolAt(file, params, std::move(reply));
}

void ArkLanguageServer::OnCrossLanguageGoToDefinition(const CrossLanguageJumpParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnCrossLanguageGoToDefinition in");
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->LocateCrossSymbolAt(params, std::move(reply));
}

bool ArkLanguageServer::AllowCompletion(const CompletionParams &params)
{
    std::stringstream log;
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));

    // check didopen was received before completion
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        CleanAndLog(log, "No didopen was received before completion, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        return false;
    }
    return true;
}

void ArkLanguageServer::Notify(const std::string &method, const ValueOrError &params)
{
    std::lock_guard<std::mutex> lock(transp.transpWriter);
    transp.Notify(method, params);
}

void ArkLanguageServer::OnCompletion(const CompletionParams &params, nlohmann::json id)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnCompletion in.");

    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }

    if (!AllowCompletion(params)) {
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };

    Server->FindCompletion(params, file, std::move(reply));
}

void ArkLanguageServer::OnSemanticTokens(const SemanticTokensParams &params, nlohmann::json id)
{
    // on textDocument/semanticTokens message
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnSemanticTokens in.");

    // check didopen was received before semanticTokens
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnSemanticTokens, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindSemanticTokensHighlight(file, std::move(reply));
}

void ArkLanguageServer::OnPrepareRename(const TextDocumentPositionParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnPrepareRename in");
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnPrepareRename, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        nlohmann::json result = nullptr;
        transp.Reply(std::move(id), ValueOrError(ValueOrErrorCheck::VALUE, result));
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->PrepareRename(file, params, std::move(reply));
}

void ArkLanguageServer::OnRename(const RenameParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnRename in");
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnRename, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->Rename(file, params, std::move(reply));
}

void ArkLanguageServer::OnBreakpoints(const TextDocumentParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnBreakpoints in");
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindBreakpoints(file, std::move(reply));
}

void ArkLanguageServer::OnCodeLens(const TextDocumentParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnCodeLens in");
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindCodeLens(file, std::move(reply));
}

std::vector<DiagnosticToken> ArkLanguageServer::GetDiagsOfCurFile(std::string file)
{
    std::vector<DiagnosticToken> value;
    std::stringstream log;
    Logger &logger = Logger::Instance();
    CleanAndLog(log, "ArkLanguageServer::GetDiagsOfCurFile in, file:" + file + ".");
    logger.LogMessage(MessageType::MSG_LOG, log.str());

    std::lock_guard<std::mutex> lock(fixItsMutex);
    auto diagToFixItsIter = fixItsMap.find(file);
    if (diagToFixItsIter == fixItsMap.end()) {
        CleanAndLog(log, "file:" + file + ", diagnostic is null.");
        logger.LogMessage(MessageType::MSG_INFO, log.str());
        return value;
    }
    const auto &diagToFixItsMap = diagToFixItsIter->second;
    for (const auto &iter : diagToFixItsMap) {
        value.push_back(iter);
    }
    return value;
}

void ArkLanguageServer::RemoveDiagOfCurPkg(const std::string& dirName)
{
    std::lock_guard<std::mutex> lock(fixItsMutex);
    std::vector<std::string> files = GetAllFilesUnderCurrentPath(dirName, CANGJIE_FILE_EXTENSION(), false);
    for (auto &iter : files) {
        LowFileName(iter);
        std::string path = JoinPath(dirName, iter);
        (void)fixItsMap.erase(path);
    }
    files = GetAllFilesUnderCurrentPath(dirName, CANGJIE_MACRO_FILE_EXTENSION(), false);
    for (auto &iter : files) {
        std::string path = JoinPath(dirName, iter);
        (void) fixItsMap.erase(path);
    }
}

void ArkLanguageServer::RemoveDiagOfCurFile(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(fixItsMutex);
    std::string normalizeFilePath = NormalizePath(filePath);
    (void)fixItsMap.erase(normalizeFilePath);
}

void ArkLanguageServer::UpdateDiagnostic(std::string file, DiagnosticToken diagToken)
{
    std::lock_guard<std::mutex> lock(fixItsMutex);
    (void)fixItsMap[file].emplace(diagToken);
}

void ArkLanguageServer::RemoveDiagnostic(std::string file, DiagnosticToken diagToken)
{
    std::lock_guard<std::mutex> lock(fixItsMutex);
    auto fileIt = fixItsMap.find(file);
    if (fileIt != fixItsMap.end()) {
        fileIt->second.erase(diagToken);
    }
}

void ArkLanguageServer::PublishDiagnostics(const PublishDiagnosticsParams &params)
{
    std::stringstream log;
    Logger &logger = Logger::Instance();
    CleanAndLog(log, "ArkLanguageServer::PublishDiagnostics in, file:" + params.uri.file + ".");
    logger.LogMessage(MessageType::MSG_LOG, log.str());

    nlohmann::json jsonParams;
    if (!ToJSON(params, jsonParams)) {
        return;
    }
    ValueOrError result(ValueOrErrorCheck::VALUE, jsonParams);
    if (MessageHeaderEndOfLine::GetIsDeveco()) {
        Notify("textDocument/extendPublishDiagnostics", result);
    } else {
        Notify("textDocument/publishDiagnostics", result);
    }
}

bool ArkLanguageServer::WhetherSupportVersionInDiag() const
{
    return clientCapabilities.textDocumentClientCapabilities.diagnosticVersionSupport;
}

void ArkLanguageServer::ReadyForDiagnostics(std::string file,
    int64_t version, std::vector<DiagnosticToken> diagnostics)
{
    if (IsInCjlibDir(file)) {
        return;
    }
    std::stringstream log;
    CleanAndLog(log, "ArkLanguageServer::ReadyForDiagnostics in, file:" + file + ".");
    Logger::Instance().LogMessage(MessageType::MSG_LOG, log.str());

    PublishDiagnosticsParams notification;
    if (WhetherSupportVersionInDiag()) {
        notification.version = version;
    }
    notification.uri.file = URI::URIFromAbsolutePath(file).ToString();
    ArkAST *arkAst = CompilerCangjieProject::GetInstance()->GetArkAST(file);
    AddDiagnosticQuickFix(diagnostics, arkAst, file);
    for (auto &diagnostic: diagnostics) {
        diagnostic.range = TransformFromIDE2Char(diagnostic.range);
        if (arkAst != nullptr) {
            UpdateRange(arkAst->tokens, diagnostic.range, *arkAst->file, false);
        }
        diagnostic.range = TransformFromChar2IDE(diagnostic.range);
    }

    notification.diagnostics = std::move(diagnostics);

    // Send a notification to the LSP client.
    PublishDiagnostics(notification);
}

// LCOV_EXCL_START
void ArkLanguageServer::AddDiagnosticQuickFix(std::vector<DiagnosticToken> &diagnostics, ArkAST *arkAst,
    std::string file)
{
    if (!arkAst) {
        return;
    }
    const std::string uri = URI::URIFromAbsolutePath(file).ToString();
    for (auto &diagnostic : diagnostics) {
        if (!diagnostic.diagFix.has_value()) {
            continue;
        }
        if (diagnostic.diagFix->addImport) {
            AddImportQuickFix(diagnostic, arkAst);
        }
        if (diagnostic.diagFix->removeImport) {
            RemoveImportQuickFix(diagnostic, arkAst, uri);
        }
        if (diagnostic.diagFix->removeUnusedSymbol) {
            RemoveUnusedSymbolQuickFix(diagnostic, arkAst, uri);
        }
    }
    ImportAllSymsCodeAction(diagnostics, uri);
    RemoveAllUnusedImportsCodeAction(diagnostics, arkAst, uri);
}

void ArkLanguageServer::ImportAllSymsCodeAction(std::vector<DiagnosticToken> &diagnostics, const std::string &uri)
{
    if (diagnostics.empty()) {
        return;
    }
    std::map<Range, std::vector<DiagnosticToken*>> groups;
    for (auto &diagnostic : diagnostics) {
        groups[diagnostic.range].push_back(&diagnostic);
    }
    for (auto &[range, diags] : groups) {
        (void)range;
        if (diags.size() <= 1) {
            continue;
        }
        std::vector<CodeAction> allImports;
        int insertIdx = -1;
        for (size_t i = 0; i < diags.size(); ++i) {
            if (!diags[i] || !NeedCollect2AllImport(*diags[i])) {
                continue;
            }
            if (insertIdx == -1) {
                insertIdx = static_cast<int>(i);
            }
            CollectCA2AllImport(allImports, diags[i]->codeActions.value());
        }
        if (allImports.size() <= 1 || !diags[insertIdx]) {
            continue;
        }
        CodeAction allImportCA;
        allImportCA.kind = CodeAction::QUICKFIX_ADD_IMPORT;
        allImportCA.title = "import all symbols";
        WorkspaceEdit edit;
        std::set<TextEdit> textEdits;
        for (const auto &ca : allImports) {
            TextEdit textEdit;
            if (!ca.edit) {
                continue;
            }
            auto& changes = ca.edit->changes;
            auto it = changes.find(uri);
            if (it != changes.end() && !it->second.empty()) {
                textEdit.range = it->second[0].range;
                textEdit.newText = it->second[0].newText;
                textEdits.insert(textEdit);
            }
        }
        std::vector<TextEdit> textEditVec(textEdits.begin(), textEdits.end());
        edit.changes[uri] = textEditVec;
        allImportCA.edit = std::move(edit);
        diags[insertIdx]->codeActions.value().insert(diags[insertIdx]->codeActions.value().begin(),
            std::move(allImportCA));
    }
}

bool ArkLanguageServer::NeedCollect2AllImport(const DiagnosticToken &diagnostic)
{
    if (!diagnostic.codeActions || diagnostic.codeActions.value().empty()) {
        return false;
    }
    int quickFixCount = 0;
    for (const auto &codeAction : diagnostic.codeActions.value()) {
        if (codeAction.kind == CodeAction::QUICKFIX_ADD_IMPORT) {
            quickFixCount++;
        }
    }
    return quickFixCount == 1;
}

void ArkLanguageServer::CollectCA2AllImport(std::vector<CodeAction> &allImports,
    const std::vector<CodeAction> &codeActions)
{
    for (const auto &codeAction : codeActions) {
        if (codeAction.kind == CodeAction::QUICKFIX_ADD_IMPORT) {
            allImports.push_back(codeAction);
            break;
        }
    }
}

void ArkLanguageServer::ReportCjoVersionErr(std::string message)
{
    nlohmann::json reply;
    reply["message"] = message;
    ValueOrError result(ValueOrErrorCheck::VALUE, reply);
    Notify("textDocument/checkHealthy", result);
}

void ArkLanguageServer::PublishCompletionTip(const CompletionTip &params)
{
    nlohmann::json reply;
    reply["uri"] = params.uri.file;
    reply["tip"] = params.tip;
    reply["position"]["line"] = params.position.line;
    reply["position"]["character"] = params.position.column;
    ValueOrError result(ValueOrErrorCheck::VALUE, reply);
    Notify("textDocument/publishCompletionTip", result);
}
// LCOV_EXCL_STOP
void ArkLanguageServer::OnSignatureHelp(const SignatureHelpParams &params, nlohmann::json id)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnSignatureHelp in.");

    // check didOpen was received before OnSignatureHelp
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didOpen was received before OnSignatureHelp, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindSignatureHelp(params, file, std::move(reply));
}

void ArkLanguageServer::OnWorkspaceSymbol(const WorkspaceSymbolParams &params, nlohmann::json id)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnWorkspaceSymbol in");
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindWorkspaceSymbols(params.query, std::move(reply));
}

void ArkLanguageServer::OnDocumentSymbol(const DocumentSymbolParams &params, nlohmann::json id)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnDocumentSymbol in");
    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindDocumentSymbol(params, reply);
}
// LCOV_EXCL_START
void ArkLanguageServer::AddImportQuickFix(DiagnosticToken &diagnostic, ArkAST *arkAst)
{
    std::string identifier = GetSubStrBetweenSingleQuote(diagnostic.message);
    if (identifier.empty() || !arkAst->packageInstance) {
        return;
    }
    HandleAddImportQuickFix(
        diagnostic, identifier, arkAst->file, &arkAst->packageInstance->importManager);
}

void ArkLanguageServer::HandleAddImportQuickFix(DiagnosticToken &diagnostic, const std::string& identifier,
    Ptr<const File> file, Cangjie::ImportManager *importManager)
{
    auto index = CompilerCangjieProject::GetInstance()->GetIndex();
    if (!index) {
        return;
    }
    if (!file || !file->package || !file->curPackage) {
        return;
    }
    // get import's pos
    int lastImportLine = 0;
    for (const auto &import : file->imports) {
        if (!import) {
            continue;
        }
        lastImportLine = std::max(import->content.rightCurlPos.line, std::max(import->importPos.line, lastImportLine));
    }
    Position pkgPos = file->package->packagePos;
    if (lastImportLine == 0 && pkgPos.line > 0) {
        lastImportLine = pkgPos.line;
    }
    Position textEditStart = {pkgPos.fileID, lastImportLine, 0};
    Range textEditRange{textEditStart, textEditStart};
    std::string curPackage = file->curPackage->fullPackageName;
    std::string curModule = CompilerCangjieProject::GetInstance()->GetModuleNameByFile(file->filePath, curPackage);
    bool includeScriptRequire = CompilerCangjieProject::GetInstance()->IsBuildScriptFile(file->filePath);
    lsp::SymbolSearchContext context{curPackage, curModule, includeScriptRequire};
    std::unordered_set<std::string> fixSet = {};
    std::vector<CodeAction> actions = {};
    std::string uri = URI::URIFromAbsolutePath(file->filePath).ToString();
    std::unordered_set<lsp::SymbolID> importedSyms = {};
    GetCurFileImportedSymbolIDs(importManager, file, importedSyms);
    index->FindImportSymsOnQuickFix(
        context, importedSyms, identifier,
        [&actions, &fixSet, &textEditRange, uri, this](const std::string &pkg, const lsp::Symbol &sym) {
            std::string fullSymName = pkg + ":" + sym.name;
            if (fixSet.count(fullSymName)) {
                return;
            } else {
                fixSet.insert(fullSymName);
            }
            if (CompletionImpl::externalImportSym.count(fullSymName)) {
                ArkLanguageServer::HandleExternalImportSym(actions, pkg, sym, textEditRange, uri);
                return;
            }
            CodeAction codeAction;
            codeAction.kind = CodeAction::QUICKFIX_ADD_IMPORT;
            codeAction.title = "import " + pkg + "." + sym.name;
            WorkspaceEdit edit;
            ark::TextEdit textEdit;
            textEdit.range = textEditRange;
            textEdit.newText = "import " + pkg + "." + sym.name + "\n";
            edit.changes[uri].push_back(textEdit);
            codeAction.edit = edit;
            actions.push_back(codeAction);
        });
    diagnostic.codeActions = actions;
}

void ArkLanguageServer::RemoveImportQuickFix(DiagnosticToken &diagnostic, ArkAST *arkAst, const std::string &uri)
{
    if (!arkAst->file) {
        return;
    }
    Range removeRange = TransformFromIDE2Char(diagnostic.range);
    // check unused import whether is inside multi-import, if yes remove this symbol and the comma that follow it.
    for (auto &importSpec : arkAst->file->imports) {
        if (!importSpec || importSpec->end.IsZero() || importSpec.get()->content.kind != ImportKind::IMPORT_MULTI) {
            continue;
        }
        if (!(importSpec->begin <= removeRange.start && importSpec->end >= removeRange.end)) {
            continue;
        }
        // remove entire multi-import if have only symbol inside multi-import
        if (importSpec->content.items.size() == 1) {
            Position multiImportEnd = importSpec->content.rightCurlPos;
            multiImportEnd.column++;
            removeRange = {importSpec->begin, multiImportEnd};
            break;
        }
        auto commaPoses = importSpec->content.commaPoses;
        for (const auto &commaPos: commaPoses) {
            if (commaPos >= removeRange.end) {
                removeRange.end = commaPos;
                removeRange.end.column++;
                break;
            }
        }
        break;
    }
    std::string diagMessage = diagnostic.message.substr(0, diagnostic.message.find_first_of(','));
    CodeAction codeAction;
    codeAction.kind = CodeAction::QUICKFIX_REMOVE_IMPORT;
    codeAction.title = "Remove " + diagMessage;
    WorkspaceEdit edit;
    TextEdit textEdit;
    textEdit.range = TransformFromChar2IDE(removeRange);
    textEdit.newText = "";
    edit.changes[uri].push_back(textEdit);
    codeAction.edit = edit;
    if (diagnostic.codeActions.has_value()) {
        diagnostic.codeActions.value().push_back(codeAction);
    } else {
        diagnostic.codeActions = {codeAction};
    }
}

void ArkLanguageServer::RemoveAllUnusedImportsCodeAction(std::vector<DiagnosticToken> &diagnostics, ArkAST *arkAst,
    const std::string &uri)
{
    if (!arkAst->file) {
        return;
    }
    int unusedImportCount = GetUnusedImportCount(diagnostics);
    if (unusedImportCount <= 1) {
        return;
    }
    std::map<Range, std::pair<int, int>> multiImportMap;
    GetMultiImportMap(multiImportMap, arkAst);
    CodeAction allUnusedImportCA;
    allUnusedImportCA.kind = CodeAction::QUICKFIX_REMOVE_IMPORT;
    allUnusedImportCA.title = "Remove all unused imports";
    std::set<Range> removeMultiImports;
    std::map<Range, std::vector<TextEdit>> multiImport2TextEdit;
    WorkspaceEdit edit = GetWorkspaceEdit(diagnostics, uri, removeMultiImports, multiImport2TextEdit, multiImportMap);
    allUnusedImportCA.edit = edit;
    if (!allUnusedImportCA.edit.has_value() || allUnusedImportCA.edit.value().changes[uri].empty()) {
        return;
    }
    for (auto &removeMultiImport : removeMultiImports) {
        if (multiImport2TextEdit.find(removeMultiImport) == multiImport2TextEdit.end()) {
            continue;
        }
        auto textEdits = multiImport2TextEdit[removeMultiImport];
        allUnusedImportCA.edit.value().changes[uri].erase(
            std::remove_if(allUnusedImportCA.edit.value().changes[uri].begin(),
                allUnusedImportCA.edit.value().changes[uri].end(), [textEdits](const TextEdit &textEdit) {
                return std::find(textEdits.begin(), textEdits.end(), textEdit) != textEdits.end();
            }), allUnusedImportCA.edit.value().changes[uri].end());
        TextEdit textEdit;
        textEdit.range = TransformFromChar2IDE(removeMultiImport);
        textEdit.newText = "";
        allUnusedImportCA.edit.value().changes[uri].push_back(textEdit);
    }
    for (auto &diagnostic : diagnostics) {
        if (!diagnostic.diagFix.has_value() || !diagnostic.diagFix->removeImport || !diagnostic.codeActions.has_value()) {
            continue;
        }
        diagnostic.codeActions.value().push_back(allUnusedImportCA);
    }
}


void ArkLanguageServer::HandleExternalImportSym(std::vector<CodeAction> &actions, const std::string &pkg,
    const lsp::Symbol &sym, Range textEditRange, const std::string &uri)
{
    if (sym.name == "Interop" && pkg == "ohos.ark_interop_macro") {
        CodeAction codeAction;
        codeAction.kind = CodeAction::QUICKFIX_ADD_IMPORT;
        codeAction.title = "import " + pkg + "." + sym.name;
        WorkspaceEdit edit;
        ark::TextEdit textEdit;
        textEdit.range = textEditRange;
        textEdit.newText = "import ohos.ark_interop_macro.*\nimport ohos.ark_interop.*\n";
        edit.changes[uri].push_back(textEdit);
        codeAction.edit = edit;
        actions.push_back(codeAction);
        return;
    }
}

struct RemoveUnusedContext {
    const std::vector<Cangjie::Token> &tokens;
    const Cangjie::AST::Node &node;
    const std::string &uri;
};

static std::string GetSymbolKindDescription(ASTKind kind, const Decl* decl = nullptr)
{
    switch (kind) {
        case ASTKind::FUNC_DECL:
            if (decl && decl->outerDecl &&
                decl->outerDecl->astKind == ASTKind::ENUM_DECL) {
                return "Enum variant";
            }
            return "Function";
        case ASTKind::VAR_DECL:
            if (decl && decl->outerDecl &&
                decl->outerDecl->astKind == ASTKind::ENUM_DECL) {
                return "Enum variant";
            }
            return "Variable";
        case ASTKind::CLASS_DECL:
            return "Class";
        case ASTKind::STRUCT_DECL:
            return "Struct";
        case ASTKind::ENUM_DECL:
            return "Enum";
        case ASTKind::INTERFACE_DECL:
            return "Interface";
        case ASTKind::TYPE_ALIAS_DECL:
            return "Type alias";
        default:
            return "Symbol";
    }
}

static bool TryMapParamRange(const FuncParam* funcParam, const Decl* decl, Range& deleteRange)
{
    auto mappedBegin = decl->curMacroCall->GetMacroCallPos(funcParam->begin);
    auto mappedEnd = decl->curMacroCall->GetMacroCallPos(funcParam->end);

    bool beginMapped = !mappedBegin.IsZero() &&
                      (mappedBegin.line != funcParam->begin.line || mappedBegin.column != funcParam->begin.column);
    bool endMapped = !mappedEnd.IsZero() &&
                      (mappedEnd.line != funcParam->end.line || mappedEnd.column != funcParam->end.column);

    if (!mappedBegin.IsZero()) {
        deleteRange.start = mappedBegin;
    }

    // If begin was mapped but end was not, calculate end based on the length
    if (beginMapped && !endMapped) {
        int length = funcParam->end.column - funcParam->begin.column;
        deleteRange.end.line = mappedBegin.line;
        deleteRange.end.column = mappedBegin.column + length;
    } else if (!mappedEnd.IsZero()) {
        deleteRange.end = mappedEnd;
    }

    return true;
}

static size_t FindParamIndex(const FuncParam* funcParam, const FuncBody* funcBody, bool& found)
{
    found = false;
    if (!funcBody || funcBody->paramLists.empty() || !funcBody->paramLists[0]) {
        return 0;
    }
    auto& params = funcBody->paramLists[0]->params;
    for (size_t i = 0; i < params.size(); i++) {
        if (params[i].get() == funcParam) {
            found = true;
            return i;
        }
    }
    return 0;
}

static void AdjustParamDeleteRangeForNext(const Decl* decl, Range& deleteRange, const FuncParam* nextParam)
{
    auto nextParamBegin = nextParam->begin;
    if (decl && decl->curMacroCall && !decl->isInMacroCall) {
        auto mappedNextBegin = decl->curMacroCall->GetMacroCallPos(nextParamBegin);
        if (!mappedNextBegin.IsZero()) {
            nextParamBegin = mappedNextBegin;
        }
    }
    deleteRange = {deleteRange.start, nextParamBegin};
}

static void AdjustParamDeleteRangeForLast(const Decl* decl,
    Range& deleteRange, const FuncParam* prevParam)
{
    auto prevCommaPos = prevParam->commaPos;
    if (decl && decl->curMacroCall && !decl->isInMacroCall) {
        auto mappedCommaPos = decl->curMacroCall->GetMacroCallPos(prevCommaPos);
        if (!mappedCommaPos.IsZero()) {
            prevCommaPos = mappedCommaPos;
        }
    }
    deleteRange = {prevCommaPos, deleteRange.end};
}

static bool ProcessFuncParamDeleteRange(const FuncParam* funcParam, Range& deleteRange,
    std::string& symbolKindDesc, const FuncBody* funcBody, const Decl* decl)
{
    symbolKindDesc = "Parameter";
    deleteRange = {funcParam->begin, funcParam->end};

    bool hasMacroCall = decl && decl->curMacroCall && !decl->isInMacroCall;
    if (hasMacroCall && !TryMapParamRange(funcParam, decl, deleteRange)) {
        return false;
    }

    bool foundIndex = false;
    size_t paramIndex = FindParamIndex(funcParam, funcBody, foundIndex);

    if (!funcBody || funcBody->paramLists.empty() || !funcBody->paramLists[0]) {
        return true;
    }

    auto& params = funcBody->paramLists[0]->params;
    if (!foundIndex || params.size() <= 1) {
        return true;
    }

    if (paramIndex < params.size() - 1) {
        AdjustParamDeleteRangeForNext(decl, deleteRange, params[paramIndex + 1].get());
    } else {
        AdjustParamDeleteRangeForLast(decl, deleteRange, params[paramIndex - 1].get());
    }

    return true;
}

static bool IsDirectMacroCall(const MacroExpandDecl* macroExpand, const Decl* decl)
{
    if (!macroExpand) {
        return false;
    }
    return macroExpand->invocation.decl.get() == decl ||
           (!decl->outerDecl && macroExpand->invocation.decl.get() == decl) ||
           (macroExpand->invocation.decl &&
            macroExpand->invocation.decl->identifier == decl->identifier &&
            macroExpand->invocation.decl->GetIdentifierPos() == decl->GetIdentifierPos());
}

static bool IsLocalVarDecl(const Decl* decl)
{
    auto varDecl = DynamicCast<const VarDecl*>(decl);
    return varDecl && !IsGlobalOrMember(*varDecl);
}

static void ApplyMacroCallMapping(const Decl* decl, Range& deleteRange)
{
    auto mappedStart = decl->curMacroCall->GetMacroCallPos(deleteRange.start);
    auto mappedEnd = decl->curMacroCall->GetMacroCallPos(deleteRange.end);
    if (!mappedStart.IsZero() && !mappedEnd.IsZero()) {
        deleteRange.start = mappedStart;
        deleteRange.end = mappedEnd;
    }
}

static void SetDeleteRangeFromAtPos(const Decl* decl, Range& deleteRange)
{
    auto inv = decl->curMacroCall->GetConstInvocation();
    if (inv && !inv->atPos.IsZero()) {
        deleteRange.start = inv->atPos;
        deleteRange.end = decl->curMacroCall->end;
    } else {
        deleteRange.start = decl->curMacroCall->begin;
        deleteRange.end = decl->curMacroCall->end;
    }
}

static void SetDeleteRangeStartFromAtPos(const Decl* decl, Range& deleteRange)
{
    auto inv = decl->curMacroCall->GetConstInvocation();
    if (inv && !inv->atPos.IsZero()) {
        deleteRange.start = inv->atPos;
        deleteRange.end = decl->end;
        auto mappedEnd = decl->curMacroCall->GetMacroCallPos(deleteRange.end);
        if (!mappedEnd.IsZero() && mappedEnd != deleteRange.end) {
            deleteRange.end = mappedEnd;
        }
    } else {
        deleteRange.start = decl->begin;
    }
}

static void HandleDirectMacroCall(const Decl* decl, Range& deleteRange)
{
    if (IsLocalVarDecl(decl)) {
        ApplyMacroCallMapping(decl, deleteRange);
    } else {
        SetDeleteRangeFromAtPos(decl, deleteRange);
    }
}

static void HandleIndirectMacroCall(const Decl* decl, Range& deleteRange)
{
    if (!decl->outerDecl) {
        if (IsLocalVarDecl(decl)) {
            ApplyMacroCallMapping(decl, deleteRange);
        } else {
            SetDeleteRangeStartFromAtPos(decl, deleteRange);
        }
    } else {
        ApplyMacroCallMapping(decl, deleteRange);
    }
}

static void HandleCurMacroCall(const Decl* decl, Range& deleteRange)
{
    auto curMacroExpand = DynamicCast<const MacroExpandDecl*>(decl->curMacroCall.get());
    if (IsDirectMacroCall(curMacroExpand, decl)) {
        HandleDirectMacroCall(decl, deleteRange);
    } else {
        HandleIndirectMacroCall(decl, deleteRange);
    }
}

static void HandleAnnotations(const Decl* decl, Range& deleteRange)
{
    auto& firstAnno = decl->annotations.front();
    if (firstAnno && firstAnno->astKind == ASTKind::ANNOTATION) {
        deleteRange.start = firstAnno->begin;
    }
}

static void HandleOuterMacroExpand(const Decl* decl, Range& deleteRange)
{
    auto outerInv = decl->outerDecl->GetConstInvocation();
    if (outerInv && !outerInv->atPos.IsZero()) {
        deleteRange.start = outerInv->atPos;
    } else {
        deleteRange.start = decl->outerDecl->begin;
    }
}

static void HandleParentMacroExpand(const Decl* decl,
    const MacroExpandDecl* parentMacroExpandDecl, Range& deleteRange)
{
    auto& pInv = parentMacroExpandDecl->invocation;
    if (pInv.atPos.IsZero()) {
        return;
    }
    auto varDecl = DynamicCast<const VarDecl*>(decl);
    if (varDecl && !IsGlobalOrMember(*varDecl)) {
        return;
    }
    deleteRange.start = pInv.atPos;
}

static void HandleEnumVariant(const Decl* decl, const ArkAST* arkAst, Range& deleteRange)
{
    int pipeColumn = -1;
    for (int i = static_cast<int>(arkAst->tokens.size()) - 1; i >= 0; --i) {
        const auto& tok = arkAst->tokens[i];
        if (tok.kind == Cangjie::TokenKind::BITOR &&
            tok.Begin().line == decl->begin.line &&
            tok.Begin().column < decl->begin.column) {
            pipeColumn = tok.Begin().column;
            break;
        }
    }
    deleteRange.start.line = decl->begin.line;
    deleteRange.start.column = (pipeColumn > 0) ? pipeColumn : decl->begin.column;
    deleteRange.end = decl->end;
}

static std::string BuildDiagQuery(const Range& diagRange)
{
    return "_ = (" + std::to_string(diagRange.start.fileID) + ", " +
        std::to_string(diagRange.start.line) + ", " + std::to_string(diagRange.start.column) + ")";
}

static bool MatchDeclFromSearchResult(const Decl* decl, const Cangjie::AST::Symbol* sym,
    const ArkAST* arkAst, Range& deleteRange)
{
    if (!sym || !sym->node) {
        return false;
    }
    auto realDecl = arkAst->FindDeclByNode(sym->node);
    if (!realDecl || realDecl->identifier != decl->identifier) {
        return false;
    }
    if (!decl->outerDecl) {
        auto inv = decl->curMacroCall->GetConstInvocation();
        if (inv && !inv->atPos.IsZero()) {
            deleteRange.start = inv->atPos;
        } else {
            deleteRange.start = realDecl->begin;
        }
        deleteRange.end = realDecl->end;
    } else {
        deleteRange.start = realDecl->begin;
        deleteRange.end = realDecl->end;
    }
    return true;
}

static bool MatchDeclFromSearchResultSimple(const Decl* decl, const Cangjie::AST::Symbol* sym,
    const ArkAST* arkAst, Range& deleteRange)
{
    if (!sym || !sym->node) {
        return false;
    }
    auto realDecl = arkAst->FindDeclByNode(sym->node);
    if (!realDecl || realDecl->identifier != decl->identifier) {
        return false;
    }
    deleteRange.start = realDecl->begin;
    deleteRange.end = realDecl->end;
    return true;
}

static bool TryResolveFromIndexWithSymbol(const Decl* decl, const Range& diagRange,
    const ArkAST* arkAst, Range& deleteRange)
{
    bool needQuery = (deleteRange.start.line != deleteRange.end.line) ||
                    (decl->outerDecl && deleteRange.end.line != diagRange.start.line);
    if (!needQuery) {
        return false;
    }

    auto syms = SearchContext(arkAst->packageInstance->ctx, BuildDiagQuery(diagRange));
    for (const auto sym : syms) {
        if (MatchDeclFromSearchResult(decl, sym, arkAst, deleteRange)) {
            return true;
        }
    }
    return false;
}

static bool TryResolveFromIndexFallback(const Decl* decl, const Range& diagRange,
    const ArkAST* arkAst, Range& deleteRange)
{
    if (deleteRange.start.line == deleteRange.end.line || !arkAst->packageInstance) {
        return false;
    }

    auto syms = SearchContext(arkAst->packageInstance->ctx, BuildDiagQuery(diagRange));
    for (const auto sym : syms) {
        if (MatchDeclFromSearchResultSimple(decl, sym, arkAst, deleteRange)) {
            return true;
        }
    }
    return false;
}

static bool TryResolveFromIndex(const Decl* decl, const Range& diagRange,
    const ArkAST* arkAst, Range& deleteRange)
{
    auto index = ark::CompilerCangjieProject::GetInstance()->GetIndex();
    if (!index || !decl->curMacroCall || decl->isInMacroCall || !arkAst) {
        return false;
    }

    auto symFromIndex = index->GetAimSymbol(*decl);
    if (!symFromIndex.location.fileUri.empty()) {
        return TryResolveFromIndexWithSymbol(decl, diagRange, arkAst, deleteRange);
    }
    return TryResolveFromIndexFallback(decl, diagRange, arkAst, deleteRange);
}

static bool IsAnnotationMacro(const Decl* decl)
{
    return !decl->annotations.empty() &&
           (decl->curMacroCall->begin.line == decl->begin.line) &&
           (decl->curMacroCall->begin.column < decl->begin.column);
}

static bool HandleAnnotationMacro(const Decl* decl, Range& deleteRange)
{
    auto inv = decl->curMacroCall->GetConstInvocation();
    if (inv && !inv->atPos.IsZero()) {
        deleteRange.start = inv->atPos;
        deleteRange.end = decl->end;
    } else {
        deleteRange.start = decl->curMacroCall->begin;
        deleteRange.end = decl->curMacroCall->end;
    }
    return true;
}

static bool ShouldHandleCurMacroCall(const Decl* decl)
{
    return decl->curMacroCall && !decl->isInMacroCall;
}

static bool ShouldHandleAnnotations(const Decl* decl)
{
    return !decl->annotations.empty();
}

static bool ShouldHandleOuterMacroExpand(const Decl* decl)
{
    return decl->outerDecl && decl->outerDecl->astKind == ASTKind::MACRO_EXPAND_DECL;
}

static bool ShouldHandleParentMacroExpand(const Decl* decl, const MacroExpandDecl* parentMacroExpandDecl)
{
    return parentMacroExpandDecl &&
           (decl->outerDecl == static_cast<const Decl*>(parentMacroExpandDecl) ||
            (!decl->outerDecl && parentMacroExpandDecl->invocation.decl.get() == decl));
}

static bool ShouldHandleEnumVariant(const Decl* decl, const ArkAST* arkAst)
{
    return decl->outerDecl &&
           decl->outerDecl->astKind == ASTKind::ENUM_DECL &&
           (decl->astKind == ASTKind::FUNC_DECL || decl->astKind == ASTKind::VAR_DECL) &&
           arkAst;
}

static bool ProcessDeclDeleteRange(const Decl* decl,
    const MacroExpandDecl* parentMacroExpandDecl, Range& deleteRange, const ArkAST* arkAst)
{
    if (ShouldHandleEnumVariant(decl, arkAst)) {
        HandleEnumVariant(decl, arkAst, deleteRange);
    }
    
    if (ShouldHandleCurMacroCall(decl)) {
        if (IsAnnotationMacro(decl)) {
            return HandleAnnotationMacro(decl, deleteRange);
        } else {
            HandleCurMacroCall(decl, deleteRange);
        }
    } else if (ShouldHandleAnnotations(decl)) {
        HandleAnnotations(decl, deleteRange);
    } else if (ShouldHandleOuterMacroExpand(decl)) {
        HandleOuterMacroExpand(decl, deleteRange);
    } else if (ShouldHandleParentMacroExpand(decl, parentMacroExpandDecl)) {
        HandleParentMacroExpand(decl, parentMacroExpandDecl, deleteRange);
    }
    return false;
}

static void ComputeDeclDeleteRange(const Decl* decl,
    const MacroExpandDecl* parentMacroExpandDecl, Range& deleteRange, const Range& diagRange, const ArkAST* arkAst)
{
    deleteRange = {decl->begin, decl->end};
    bool skipTryResolveFromIndex = ProcessDeclDeleteRange(decl, parentMacroExpandDecl, deleteRange, arkAst);
    if (!skipTryResolveFromIndex) {
        TryResolveFromIndex(decl, diagRange, arkAst, deleteRange);
    }
}

static void AddRemoveUnusedCodeAction(DiagnosticToken &diagnostic,
    const RemoveUnusedContext &ctx, const Range &deleteRange,
    const std::string &symbolName, const std::string &symbolKindDesc)
{
    std::string lowerKindDesc = symbolKindDesc;
    if (!lowerKindDesc.empty()) {
        lowerKindDesc[0] = std::tolower(static_cast<unsigned char>(lowerKindDesc[0]));
    }

    CodeAction codeAction;
    codeAction.kind = CodeAction::QUICKFIX_REMOVE_UNUSED_SYMBOL;
    codeAction.title = "Remove unused " + lowerKindDesc + " '" + symbolName + "'";

    WorkspaceEdit edit;
    TextEdit textEdit;
    Range ideRange = deleteRange;
    PositionUTF8ToIDE(ctx.tokens, ideRange.start, ctx.node);
    PositionUTF8ToIDE(ctx.tokens, ideRange.end, ctx.node);
    textEdit.range = TransformFromChar2IDE(ideRange);
    textEdit.newText = "";
    edit.changes[ctx.uri].push_back(textEdit);
    codeAction.edit = edit;

    if (diagnostic.codeActions.has_value()) {
        diagnostic.codeActions.value().push_back(codeAction);
    } else {
        diagnostic.codeActions = {codeAction};
    }
}

struct UnusedSymbolInfo {
    bool found = false;
    std::string symbolName;
    std::string symbolKindDesc;
    Range deleteRange;
};

static bool MatchFuncParamAtPos(const FuncParam* funcParam, const Range& diagRange,
    const FuncBody* currentFuncBody, UnusedSymbolInfo& info)
{
    auto identifierPos = funcParam->GetIdentifierPos();
    auto outerDecl = funcParam->outerDecl.get();

    auto comparePos = identifierPos;
    if (outerDecl && outerDecl->curMacroCall) {
        auto mappedPos = outerDecl->curMacroCall->GetMacroCallPos(identifierPos);
        if (!mappedPos.IsZero()) {
            comparePos = mappedPos;
        }
    }

    if (comparePos.line != diagRange.start.line || comparePos.column != diagRange.start.column) {
        return false;
    }

    info.symbolName = std::string(funcParam->identifier);
    return ProcessFuncParamDeleteRange(funcParam, info.deleteRange, info.symbolKindDesc, currentFuncBody, outerDecl);
}

// Structure to hold matched declaration info for unused symbol quickfix
struct MatchedDecl {
    const Decl* decl;
    Ptr<const MacroExpandDecl> parentMacro;
    bool isUserDecl;   // true if not inside a macro call context
    bool nameMatches;  // true if name matches the hint
};

// Select the best matched declaration using priority: name match > user-declared > first match
static UnusedSymbolInfo SelectBestMatchedDecl(const std::vector<MatchedDecl>& matchedDecls,
    const Range& diagRange, const ArkAST* arkAst)
{
    UnusedSymbolInfo info;
    info.deleteRange = diagRange;

    if (matchedDecls.empty()) {
        return info;
    }

    // First priority: exact name match (from diagnostic message)
    for (const auto& match : matchedDecls) {
        if (match.nameMatches) {
            info.symbolName = std::string(match.decl->identifier);
            info.symbolKindDesc = GetSymbolKindDescription(match.decl->astKind, match.decl);
            ComputeDeclDeleteRange(match.decl, match.parentMacro, info.deleteRange, diagRange, arkAst);
            info.found = true;
            return info;
        }
    }

    // Second priority: user-declared symbol (not inside macro call)
    for (const auto& match : matchedDecls) {
        if (match.isUserDecl) {
            info.symbolName = std::string(match.decl->identifier);
            info.symbolKindDesc = GetSymbolKindDescription(match.decl->astKind, match.decl);
            ComputeDeclDeleteRange(match.decl, match.parentMacro, info.deleteRange, diagRange, arkAst);
            info.found = true;
            return info;
        }
    }

    // Fallback: use the first match (macro-generated)
    const auto& firstMatch = matchedDecls[0];
    info.symbolName = std::string(firstMatch.decl->identifier);
    info.symbolKindDesc = GetSymbolKindDescription(firstMatch.decl->astKind, firstMatch.decl);
    ComputeDeclDeleteRange(firstMatch.decl, firstMatch.parentMacro, info.deleteRange, diagRange, arkAst);
    info.found = true;

    return info;
}

// Context for collecting matching declarations during AST walk
struct UnusedSymbolWalkContext {
    const FuncBody* currentFuncBody = nullptr;
    Ptr<const MacroExpandDecl> parentMacroExpandDecl = nullptr;
    std::vector<MatchedDecl> matchedDecls;
    UnusedSymbolInfo info;
    bool foundFuncParam = false;
};

// Process a single node during AST walk for unused symbol search
static VisitAction ProcessNodeForUnusedSymbol(Ptr<const Node> node, const Range& diagRange,
    const std::string& symbolNameHint, UnusedSymbolWalkContext& ctx)
{
    if (auto funcBody = DynamicCast<const FuncBody*>(node)) {
        ctx.currentFuncBody = funcBody;
    }

    if (auto macroExpand = DynamicCast<const MacroExpandDecl*>(node)) {
        ctx.parentMacroExpandDecl = macroExpand;
    }

    if (auto funcParam = DynamicCast<const FuncParam*>(node)) {
        if (MatchFuncParamAtPos(funcParam, diagRange, ctx.currentFuncBody, ctx.info)) {
            ctx.info.found = true;
            ctx.foundFuncParam = true;
            return VisitAction::STOP_NOW;
        }
        return VisitAction::WALK_CHILDREN;
    }

    auto decl = DynamicCast<const Decl*>(node);
    if (!decl) {
        return VisitAction::WALK_CHILDREN;
    }

    auto identifierPos = decl->GetIdentifierPos();
    bool isUserDecl = !decl->isInMacroCall;
    bool nameMatches = !symbolNameHint.empty() && decl->identifier == symbolNameHint;

    auto comparePos = identifierPos;
    if (!isUserDecl && decl->curMacroCall) {
        auto mappedPos = decl->curMacroCall->GetMacroCallPos(identifierPos);
        if (!mappedPos.IsZero()) {
            comparePos = mappedPos;
        }
    }

    if (comparePos.line != diagRange.start.line ||
        comparePos.column != diagRange.start.column) {
        return VisitAction::WALK_CHILDREN;
    }

    ctx.matchedDecls.push_back({decl, ctx.parentMacroExpandDecl, isUserDecl, nameMatches});
    return VisitAction::WALK_CHILDREN;
}

static UnusedSymbolInfo FindUnusedSymbol(const ArkAST* arkAst, const Range& diagRange,
    const std::string& symbolNameHint = "")
{
    UnusedSymbolWalkContext ctx;
    ctx.info.deleteRange = diagRange;

    std::function<VisitAction(Ptr<const Node>)> finder = [&](Ptr<const Node> node) -> VisitAction {
        return ProcessNodeForUnusedSymbol(node, diagRange, symbolNameHint, ctx);
    };

    ConstWalker(arkAst->file, finder).Walk();

    if (ctx.foundFuncParam) {
        return ctx.info;
    }

    return SelectBestMatchedDecl(ctx.matchedDecls, diagRange, arkAst);
}

void ArkLanguageServer::RemoveUnusedSymbolQuickFix(DiagnosticToken &diagnostic, ArkAST *arkAst, const std::string &uri)
{
    if (!arkAst || !arkAst->file) {
        return;
    }

    diagnostic.range.start.fileID = arkAst->fileID;
    diagnostic.range.end.fileID = arkAst->fileID;

    Range diagRange = TransformFromIDE2Char(diagnostic.range);

    // Extract symbol name from diagnostic message to avoid position mapping issues
    std::string symbolNameFromDiag;
    size_t pos1 = diagnostic.message.find("'");
    if (pos1 != std::string::npos) {
        size_t pos2 = diagnostic.message.find("'", pos1 + 1);
        if (pos2 != std::string::npos) {
            symbolNameFromDiag = diagnostic.message.substr(pos1 + 1, pos2 - pos1 - 1);
        }
    }

    auto info = FindUnusedSymbol(arkAst, diagRange, symbolNameFromDiag);
    if (!info.found) {
        return;
    }

    RemoveUnusedContext ctx{arkAst->tokens, *arkAst->file, uri};
    AddRemoveUnusedCodeAction(diagnostic, ctx, info.deleteRange, info.symbolName, info.symbolKindDesc);
}

// LCOV_EXCL_STOP
void ArkLanguageServer::OnOverrideMethods(const OverrideMethodsParams &params, nlohmann::json id)
{
    Logger& logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "ArkLanguageServer::OnOverrideMethods in");

    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before OnOverrideMethods, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(id);
        return;
    }

    auto reply = [id, this](ValueOrError result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), std::move(result));
    };
    Server->FindOverrideMethods(file, params, std::move(reply));
}

CodeAction toCodeAction(const ArkServer::TweakRef &T, const std::string &uri, Range range)
{
    CodeAction CA;
    CA.title = T.title;
    CA.kind = T.kind;
    Command command;
    command.title = T.title;
    command.command = Command::APPLY_EDIT_COMMAND;
    ExecutableRange args;
    args.tweakId = T.id;
    args.uri = uri;
    args.range = range;
    args.extraOptions = T.extraOptions;
    command.arguments.insert(args);
    CA.command = std::move(command);
    return CA;
}

void ArkLanguageServer::OnCodeAction(const CodeActionParams &params, nlohmann::json id)
{
    Trace::Log("ArkLanguageServer::OnCodeAction in");

    auto reply = [id, this](const ValueOrError &result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), result);
    };

    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before CodeAction, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(id);
        return;
    }
    std::string uri = params.textDocument.uri.file;
    Range range;
    range.start = Cangjie::Position{
        static_cast<unsigned int>(0),
        params.range.start.line,
        params.range.start.column
    };
    range.end = Cangjie::Position{
        static_cast<unsigned int>(0),
        params.range.end.line,
        params.range.end.column
    };

    auto consumeActions = [reply = std::move(reply), uri, params](
                              const std::vector<ArkServer::TweakRef> &tweaks) mutable {
        std::vector<CodeAction> actions;
        for (const auto &T : tweaks) {
            // toCodeAction():Transforms a tweak into a code action that would apply it if executed.
            actions.push_back(toCodeAction(T, uri, params.range));
        }
        nlohmann::json results;
        for (auto &action : actions) {
            nlohmann::json temp;
            ToJSON(action, temp);
            results.push_back(temp);
        }
        reply(ValueOrError(ValueOrErrorCheck::VALUE, results));
    };

    Server->EnumerateTweaks(file, range, std::move(consumeActions));
}

void ArkLanguageServer::OnCommand(const ExecuteCommandParams &params,  nlohmann::json id)
{
    Trace::Log("ArkLanguageServer::OnCommand in");

    auto reply = [id, this](const ValueOrError &result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), result);
    };

    if (params.arguments.empty()) {
        ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
        reply(value);
        return;
    }

    TweakArgs args;
    if (params.command == Command::APPLY_EDIT_COMMAND && FromJSON(params.arguments[0], args)) {
        OnCommandApplyTweak(args, id);
    }
}

void ArkLanguageServer::OnCommandApplyTweak(const TweakArgs &args, nlohmann::json id)
{
    Trace::Log("ArkLanguageServer::OnCommandApplyTweak in");

    auto reply = [id, this](const ValueOrError &result) mutable {
        std::lock_guard<std::mutex> lock(transp.transpWriter);
        transp.Reply(std::move(id), result);
    };

    std::string file = FileStore::NormalizePath(URI::Resolve(args.file.file));
    if (!CheckFileInCangjieProject(file)) {
        ReplyError(id);
        return;
    }
    DocCache::Doc doc = DocMgr.GetDoc(file);
    if (doc.version == -1) {
        std::stringstream log;
        CleanAndLog(log, "No didopen was received before CommandApplyTweak, file:" + file);
        Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
        ReplyError(id);
        return;
    }
    if (args.tweakID == "ExtractVariable") {
        auto buffer = CompilerCangjieProject::GetInstance()->GetContentByFile(file);
        auto contents = GetContentsByFile(file);
        std::regex reg("\r\n");
        std::string replaceStr = "\n";
        std::string bufferResult = std::regex_replace(buffer, reg, replaceStr);
        std::string contentsResult = std::regex_replace(contents, reg, replaceStr);
        if (!buffer.empty() && bufferResult != contentsResult) {
            CompilerCangjieProject::GetInstance()->CompilerOneFile(file, contents);
        }
    }

    auto fileId = CompilerCangjieProject::GetInstance()->GetFileID(args.file.file);
    if (!fileId) {
        ReplyTweakNoEdit(reply);
        return;
    }
    Range range = BuildTweakSelectionRange(fileId.value_or(0), args);

    auto action = [this, reply = std::move(reply)](Tweak::Effect effect) mutable {
        if (effect.applyEdits.empty() && effect.documentChanges.empty()) {
            ReplyTweakNoEdit(reply);
            return;
        }
        ReplyTweakNoEdit(reply);
        ValueOrError value = BuildTweakWorkspaceEditValue(std::move(effect));
        Notify("workspace/applyEdit", value);
    };
    Server->ApplyTweak(file, range, args.tweakID, args.extraOptions, std::move(action));
}

int ArkLanguageServer::GetUnusedImportCount(std::vector<DiagnosticToken> &diagnostics)
{
    int unusedImportCount = 0;
    for (auto &diagnostic : diagnostics) {
        if (!diagnostic.diagFix.has_value() || !diagnostic.diagFix->removeImport || !diagnostic.codeActions.has_value()) {
            continue;
        }
        unusedImportCount++;
    }
    return unusedImportCount;
}

void ArkLanguageServer::GetMultiImportMap(std::map<Range, std::pair<int, int>>& multiImportMap, ArkAST *arkAst)
{
    for (auto &importSpec : arkAst->file->imports) {
        if (!importSpec || importSpec->end.IsZero() || importSpec.get()->content.kind != ImportKind::IMPORT_MULTI) {
            continue;
        }
        Position multiImportEnd = importSpec->content.rightCurlPos;
        multiImportEnd.column++;
        Range multiImportRange = {importSpec->begin, multiImportEnd};
        multiImportMap[multiImportRange] = {importSpec->content.items.size(), 0};
    }
}

WorkspaceEdit ArkLanguageServer::GetWorkspaceEdit(std::vector<DiagnosticToken> &diagnostics,
    const std::string &uri, std::set<Range>& removeMultiImports,
    std::map<Range, std::vector<TextEdit>>& multiImport2TextEdit,
    std::map<Range, std::pair<int, int>>& multiImportMap)
{
    WorkspaceEdit edit;
    for (auto &diagnostic : diagnostics) {
        if (!diagnostic.diagFix.has_value() || !diagnostic.diagFix->removeImport || !diagnostic.codeActions.has_value()) {
            continue;
        }
        TextEdit textEdit;
        for (auto &ca : diagnostic.codeActions.value()) {
            if (ca.kind == CodeAction::QUICKFIX_REMOVE_IMPORT && ca.edit.has_value()
                && !ca.edit.value().changes[uri].empty()) {
                edit.changes[uri].push_back(ca.edit->changes[uri][0]);
                textEdit = ca.edit->changes[uri][0];
                break;
            }
        }
        if (textEdit.range.end.IsZero()) {
            continue;
        }
        for (auto &multiImport : multiImportMap) {
            Range diagRange = TransformFromIDE2Char(diagnostic.range);
            if (!(multiImport.first.start <= diagRange.start && multiImport.first.end >= diagRange.end)) {
                continue;
            }
            multiImport2TextEdit[multiImport.first].push_back(textEdit);
            multiImportMap[multiImport.first] = {multiImport.second.first, multiImport.second.second + 1};
            if (multiImport.second.first == multiImport.second.second) {
                removeMultiImports.insert(multiImport.first);
            }
        }
    }
    return edit;
}
} // namespace ark
