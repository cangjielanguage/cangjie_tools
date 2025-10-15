// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ArkLanguageServer.h"
#include <iostream>
#include <map>
#include <set>
#include <mutex>
#include <utility>
#include "capabilities/semanticHighlight/SemanticTokensAdaptor.h"
#include "capabilities/shutdown/Shutdown.h"

namespace ark {
using namespace Cangjie;
using namespace Cangjie::FileUtil;
using namespace CONSTANTS;
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
        auto intID = id.get<int>();
        if (intID != 0) {
            std::lock_guard<std::mutex> mutex(callMutex);
            // Find a corresponding callback for the request ID;
            for (size_t index = 0; index < replyCallbacks.size(); ++index) {
                if (replyCallbacks[index].first == intID) {
                    replyHandler = std::move(replyCallbacks[index].second);
                    // remove the entry
                    (void)replyCallbacks.erase(replyCallbacks.begin() + static_cast<long long>(index));
                    break;
                }
            }
        }

        if (!replyHandler) {
            // No callback being found, use a default log callback.
            CleanAndLog(log, "received a reply with ID:" + std::to_string(id.get<int>()) +
                                 ", but there was no such call");
            logger.LogMessage(MessageType::MSG_WARNING, log.str());
            return LSPRet::SUCCESS;
        }

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
        return LSPRet::SUCCESS;
    }

    // Bind an LSP method name to a call.
    template<typename Param>
    void Bind(const char *method, void (ArkLanguageServer::*handler)(const Param &, nlohmann::json))
    {
        calls[method] = [method, handler, this](const nlohmann::json &rawParams, const nlohmann::json &id) {
            Param param;
            if (!FromJSON(rawParams, param)) {
                std::stringstream log;
                log << "Failed to decode request or request not meet requirement, method:" << method;
                Logger::Instance().LogMessage(MessageType::MSG_WARNING, log.str());
                return;
            }
            (server.*handler)(param, id);
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
    if (!MessageHeaderEndOfLine::GetIsDeveco()) {
        MsgHandler->Bind("textDocument/codeLens", &ArkLanguageServer::OnCodeLens);
    }
    syncKind = TextDocumentSyncKind::SK_INCREMENTAL;
}

ArkLanguageServer::~ArkLanguageServer() {}

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
    CompilerCangjieProject::GetInstance()->UpdateBuffCache(file);
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
            fileVec = CompilerCangjieProject::GetInstance()->GetFilesInPkg(file); // multi folder
        } else if (event.type == FileChangeType::CREATED && CheckIsDirectory(file) &&
                CheckFileInCangjieProject(filePath)) {
            fileVec = GetAllFilesUnderCurrentPath(file, CANGJIE_FILE_EXTENSION, false);
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

bool ArkLanguageServer::CheckIsDirectory(const std::string &dirPath, bool isDelete) const
{
    if (dirPath.empty()) {
        return false;
    }

    struct stat buffer = {0};
    std::string realPath = PathWindowsToLinux(dirPath);
    std::string file = realPath;
    if (isDelete) {
        auto res = realPath.find_last_of('/');
        if (res != std::string::npos) {
            file = realPath.substr(0, res);
        }
    }
    return (stat(file.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
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

bool ArkLanguageServer::CheckFileInCangjieProject(const std::string &filePath, bool ignoreMacro) const
{
    if (filePath.empty() || ignoreMacro && Cangjie::FileUtil::HasExtension(filePath, CANGJIE_MACRO_FILE_EXTENSION)) {
        return false;
    }
    return CompilerCangjieProject::GetInstance()->GetCangjieFileKind(filePath).first != CangjieFileKind::MISSING;
}

void ArkLanguageServer::RemoveDocByFile(const std::string &file)
{
    DocMgr.RemoveDoc(file);
}

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
    std::vector<std::string> files = GetAllFilesUnderCurrentPath(dirName, CANGJIE_FILE_EXTENSION, false);
    for (auto &iter : files) {
        LowFileName(iter);
        std::string path = JoinPath(dirName, iter);
        (void)fixItsMap.erase(path);
    }
    files = GetAllFilesUnderCurrentPath(dirName, CANGJIE_MACRO_FILE_EXTENSION, false);
    for (auto &iter : files) {
        std::string path = JoinPath(dirName, iter);
        (void) fixItsMap.erase(path);
    }
}

void ArkLanguageServer::UpdateDiagnostic(std::string file, DiagnosticToken diagToken)
{
    std::lock_guard<std::mutex> lock(fixItsMutex);
    (void)fixItsMap[file].emplace(diagToken);
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
        notification.version.value() = version;
    }
    notification.uri.file = URI::URIFromAbsolutePath(file).ToString();
    ArkAST *arkAst = CompilerCangjieProject::GetInstance()->GetArkAST(file);
    for (auto &diagnostic: diagnostics) {
        diagnostic.range = TransformFromIDE2Char(diagnostic.range);
        if (arkAst != nullptr) {
            UpdateRange(arkAst->tokens, diagnostic.range, *arkAst->file, false);
        }
        diagnostic.range = TransformFromChar2IDE(diagnostic.range);
        if (MessageHeaderEndOfLine::GetIsDeveco()) {
            AutoImportQuickFixPrepare(diagnostic, arkAst);
        }
    }
    if (arkAst && arkAst->file) {
        AddAllImportCodeAction(diagnostics, URI::URIFromAbsolutePath(arkAst->file->filePath).ToString());
    }

    notification.diagnostics = std::move(diagnostics);

    // Send a notification to the LSP client.
    PublishDiagnostics(notification);
}

void ArkLanguageServer::AddAllImportCodeAction(std::vector<DiagnosticToken> &diagnostics, const std::string& uri)
{
    if (diagnostics.empty()) {
        return;
    }
    std::map<Range, std::vector<DiagnosticToken*>> groups;
    for (auto &diagnostic : diagnostics) {
        groups[diagnostic.range].push_back(&diagnostic);
    }
    for (auto &[range, diags] : groups) {
        if (diags.size() <= 1) {
            continue;
        }
        std::vector<CodeAction> allImports;
        int insertIdx = -1;
        for (int i = 0; i < diags.size(); ++i) {
            if (!diags[i] || !NeedCollect2AllImport(*diags[i])) {
                continue;
            }
            if (insertIdx == -1) {
                insertIdx = i;
            }
            CollectCA2AllImport(allImports, diags[i]->codeActions.value());
        }
        if (allImports.size() <= 1 || !diags[insertIdx]) {
            continue;
        }
        CodeAction allImportCA;
        allImportCA.kind = "fix auto import";
        allImportCA.title = "import all packages";
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
        if (codeAction.kind == "fix auto import") {
            quickFixCount++;
        }
    }
    return quickFixCount == 1;
}

void ArkLanguageServer::CollectCA2AllImport(std::vector<CodeAction> &allImports,
    const std::vector<CodeAction> &codeActions)
{
    for (const auto &codeAction : codeActions) {
        if (codeAction.kind == "fix auto import") {
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

void ArkLanguageServer::AutoImportQuickFixPrepare(DiagnosticToken &diagnostic, ArkAST *arkAst)
{
    if (!arkAst || !(diagnostic.diaFix.has_value() && diagnostic.diaFix->isAutoImport)) {
        return;
    }
    Position diagPos = PosFromIDE2Char(diagnostic.range.start);
    int index = arkAst->GetCurTokenByPos(diagPos, 0,
        static_cast<int>(arkAst->tokens.size()) - 1);
    std::string identifier = GetSubStrBetweenSingleQuote(diagnostic.message);
    if (identifier.empty() || !arkAst->packageInstance) {
        return;
    }
    AddAutoImportQuickFix(
        diagnostic, identifier, arkAst->file, &arkAst->packageInstance->importManager);
}

void ArkLanguageServer::AddAutoImportQuickFix(DiagnosticToken &diagnostic, const std::string& identifier,
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
    std::string curModule = SplitFullPackage(curPackage).first;
    std::unordered_set<std::string> fixSet = {};
    std::vector<CodeAction> actions = {};
    std::string uri = URI::URIFromAbsolutePath(file->filePath).ToString();
    std::unordered_set<lsp::SymbolID> importedSyms = {};
    GetCurFileImportedSymbolIDs(importManager, file, importedSyms);
    index->FindImportSymsOnQuickFix(
        curPackage, curModule, importedSyms, identifier,
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
            codeAction.kind = "fix auto import";
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

void ArkLanguageServer::HandleExternalImportSym(std::vector<CodeAction> &actions, const std::string &pkg,
    const lsp::Symbol &sym, Range textEditRange, const std::string &uri)
{
    if (sym.name == "Interop" && pkg == "ohos.ark_interop_macro") {
        CodeAction codeAction;
        codeAction.kind = "fix auto import";
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
    int fileId = CompilerCangjieProject::GetInstance()->GetFileID(file);
    if (fileId < 0) {
        ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
        reply(value);
        return;
    }
    Range range;
    range.start = Cangjie::Position{
        static_cast<unsigned int>(fileId),
        params.range.start.line,
        params.range.start.column
    };
    range.end = Cangjie::Position{
        static_cast<unsigned int>(fileId),
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

    int fileId = CompilerCangjieProject::GetInstance()->GetFileID(args.file.file);
    if (fileId < 0) {
        ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
        reply(value);
        return;
    }
    Range range;
    range.start = Cangjie::Position{
        static_cast<unsigned int>(fileId),
        args.selection.start.line,
        args.selection.start.column
    };
    range.end = Cangjie::Position{
        static_cast<unsigned int>(fileId),
        args.selection.end.line,
        args.selection.end.column
    };

    auto action = [this, reply = std::move(reply)](Tweak::Effect effect) mutable {
        if (effect.applyEdits.empty()) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        ValueOrError commandValue(ValueOrErrorCheck::VALUE, nullptr);
        reply(commandValue);

        ApplyWorkspaceEditParams editParams;
        editParams.edit.changes = std::move(effect.applyEdits);
        nlohmann::json res;
        ToJSON(editParams, res);

        ValueOrError value(ValueOrErrorCheck::VALUE, res);
        Notify("workspace/applyEdit", value);
        return;
    };
    Server->ApplyTweak(file, range, args.tweakID, args.extraOptions, std::move(action));
}
} // namespace ark
