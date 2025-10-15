// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ArkServer.h"
#include <cangjie/Utils/ConstantsUtils.h>
#include <cangjie/Utils/FileUtil.h>
#include <string>
#include <utility>

#include "capabilities/definition/CrossLanguangeDefinition.h"
#include "capabilities/documentSymbol/DocumentSymbolImpl.h"
#include "capabilities/hover/HoverImpl.h"
#include "capabilities/semanticHighlight/SemanticTokensAdaptor.h"
#include "capabilities/signatureHelp/SignatureHelpImpl.h"
#include "capabilities/typeHierarchy/TypeHierarchyImpl.h"
#include "capabilities/workspaceSymbol/FindSymbols.h"
#include "common/Constants.h"

#undef INTERFACE

namespace {
bool Contains(const Cangjie::StringPart &str, Cangjie::Position pos)
{
    return str.begin.column < pos.column < str.begin.column + str.value.size();
}
};

namespace ark {
using namespace Cangjie;
bool CompareCallHierarchyOutgoingCall(const CallHierarchyOutgoingCall &letf, const CallHierarchyOutgoingCall &right)
{
    return letf.fromRanges[0].start < right.fromRanges[0].start;
}

ArkServer::ArkServer(Callbacks *callbacks) : callback(callbacks)
{
    arkScheduler = std::make_unique<ark::ArkScheduler>(callback);
    arkSchedulerOfComplete = std::make_unique<ark::ArkScheduler>(callback);
    arkSchedulerOfSignature = std::make_unique<ark::ArkScheduler>(callback);
}

void ArkServer::FindSemanticTokensHighlight(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [file, reply = std::move(reply)](const InputsAndAST &inputAST) {
        SemanticTokens result {};
        // To avoid queue task in runWithAST crashing
        if (inputAST.ast == nullptr || !CompilerCangjieProject::GetInstance()->FileHasSemaCache(file)) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        SemanticTokensAdaptor::FindSemanticTokens(*(inputAST.ast), result, inputAST.ast->fileID);
        nlohmann::json jsonValue;
        // Wrapper for the packet to sent
        for (auto &iter : result.data) {
            (void)jsonValue["data"].push_back(iter);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("SemanticTokens", file, action);
}


void ArkServer::FindDocumentHighlights(const std::string &file, const TextDocumentPositionParams &params,
                                       const Callback<ValueOrError> &reply) const
{
    auto action = [file, params, reply = std::move(reply), this](const InputsAndAST& inputAST) {
        Cangjie::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        std::set<DocumentHighlight> result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }

        DocumentHighlightImpl::FindDocumentHighlights(*(inputAST.ast), result, pos);
        
        if (inputAST.useASTCache) {
            std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(file);
            UpdateModifierDiag(inputAST, diagnostics);
            callback->ReadyForDiagnostics(file, inputAST.inputs.version, diagnostics);
        }

        nlohmann::json jsonValue;
        for (auto &iter : result) {
            nlohmann::json temp;
            temp["kind"] = static_cast<int>(iter.kind);
            temp["range"]["start"]["line"] = iter.range.start.line;
            temp["range"]["start"]["character"] = iter.range.start.column;
            temp["range"]["end"]["line"] = iter.range.end.line;
            temp["range"]["end"]["character"] = iter.range.end.column;
            (void)jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("Highlights", file, action);
}

void ArkServer::FindTypeHierarchys(const std::string &file, const TextDocumentPositionParams &params,
                                   const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Cangjie::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        TypeHierarchyItem result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        TypeHierarchyImpl::FindTypeHierarchyImpl(*(inputAST.ast), result, pos);
        nlohmann::json jsonValue;
        nlohmann::json temp;
        if (result.kind != SymbolKind::FILE && ToJSON(result, temp)) {
            (void) jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("TypeHierarchy", file, action);
}

void ArkServer::FindSuperTypes(const std::string &file, const TypeHierarchyItem &params,
                               const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply = std::move(reply)](const InputsAndAST &inputAST) {
        std::vector<TypeHierarchyItem> results;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        TypeHierarchyImpl::FindSuperTypesImpl(results, params);
        nlohmann::json jsonValue;
        for (const auto &result: results) {
            nlohmann::json temp;
            if (ToJSON(result, temp)) {
                (void) jsonValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("SuperTypes", file, action);
}

void ArkServer::FindSubTypes(const std::string &file, const TypeHierarchyItem &params,
                             const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply = std::move(reply)](const InputsAndAST &inputAST) {
        std::vector<TypeHierarchyItem> results;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        TypeHierarchyImpl::FindSubTypesImpl(results, params);
        nlohmann::json jsonValue;
        for (const auto &item: results) {
            nlohmann::json temp;
            if (ToJSON(item, temp)) {
                (void) jsonValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("SubTypes", file, action);
}

void ArkServer::FindCallHierarchys(const std::string &file, const TextDocumentPositionParams &params,
                                   const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Cangjie::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        CallHierarchyItem result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        CallHierarchyImpl::FindCallHierarchyImpl(*(inputAST.ast), result, pos);
        nlohmann::json jsonValue;
        nlohmann::json temp;
        if (result.kind != SymbolKind::FILE && ToJSON(result, temp)) {
            (void) jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("CallHierarchy", file, action);
}

void ArkServer::FindOnIncomingCalls(const std::string &file, const CallHierarchyItem &params,
                                    const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply = std::move(reply)](const InputsAndAST &inputAST) {
        std::vector<CallHierarchyIncomingCall> results;
        CallHierarchyImpl::FindOnIncomingCallsImpl(results, params);
        nlohmann::json jsValue;
        for (const auto &result: results) {
            nlohmann::json temp;
            if (ToJSON(result, temp)) {
                (void) jsValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsValue);
        reply(value);
    };
    arkScheduler->RunWithAST("OnIncomingCalls", file, action);
}

void ArkServer::FindOnOutgoingCalls(const std::string &file, const CallHierarchyItem &params,
                                    const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply = std::move(reply)](const InputsAndAST &inputAST) {
        std::vector<CallHierarchyOutgoingCall> results;
        CallHierarchyImpl::FindOnOutgoingCallsImpl(results, params);
        std::sort(results.begin(), results.end(), CompareCallHierarchyOutgoingCall);
        nlohmann::json jsnValue;
        for (const auto &item: results) {
            nlohmann::json temp;
            if (ToJSON(item, temp)) {
                (void) jsnValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsnValue);
        reply(value);
    };
    arkScheduler->RunWithAST("OnOutgoingCalls", file, action);
}

void ArkServer::FindReferences(const std::string &file,
                               const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST& inputAST) {
        Cangjie::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        ReferencesResult result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        FindReferencesImpl::FindReferences(*(inputAST.ast), result, pos);
        nlohmann::json jsonValue;
        for (auto &iter : result.References) {
            nlohmann::json temp;
            temp["uri"] = iter.uri.file;
            temp["range"]["start"]["line"] = iter.range.start.line;
            temp["range"]["start"]["character"] = iter.range.start.column;
            temp["range"]["end"]["line"] = iter.range.end.line;
            temp["range"]["end"]["character"] = iter.range.end.column;
            (void)jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("References", file, action);
}

void ArkServer::FindFileReferences(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [file, reply = std::move(reply), this](const InputsAndAST& inputAST) {
        ReferencesResult result;
        std::string unixPath = PathWindowsToLinux(file);
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        FindReferencesImpl::FindFileReferences(*(inputAST.ast), result);
        nlohmann::json jsonValue;
        for (auto &iter : result.References) {
            nlohmann::json temp;
            if (unixPath.compare(URI::Resolve(iter.uri.file)) == 0) {
                continue;
            }
            temp["uri"] = iter.uri.file;
            temp["range"]["start"]["line"] = iter.range.start.line;
            temp["range"]["start"]["character"] = iter.range.start.column;
            temp["range"]["end"]["line"] = iter.range.end.line;
            temp["range"]["end"]["character"] = iter.range.end.column;
            (void)jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("FileReferences", file, action);
}

void ArkServer::FindWorkspaceSymbols(const std::string &query, const Callback<ValueOrError> &reply) const
{
    auto action = [query, reply = std::move(reply)](const InputsAndAST &) {
        auto result = lsp::GetWorkspaceSymbols(query);
        nlohmann::json jsonValue;
        for (auto &symbol : result) {
            nlohmann::json temp;
            if (ToJSON(symbol, temp)) {
                (void) jsonValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    std::string file;
    arkScheduler->RunWithAST("Symbol", file, action);
}

void ArkServer::FindHover(const std::string &file,
                          const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) mutable {
        Cangjie::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        Hover result = Hover();
        nlohmann::json jsonValue;
        auto nullValueReply = [reply]() {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
        };
        if (inputAST.ast == nullptr) {
            nullValueReply();
            return;
        }
        int ret = HoverImpl::FindHover(*(inputAST.ast), result, pos);
        if (ret == -1) {
            nullValueReply();
            return;
        }
        std::stringstream temp;
        for (auto &iter : result.markedString) {
            temp << iter;
        }
        if (temp.str().empty()) { // sometimes we dont have valid hover message
            nullValueReply();
            return;
        }
        jsonValue["contents"]["language"] = "Cangjie";
        jsonValue["contents"]["value"] = temp.str();

        jsonValue["range"]["start"]["line"] = result.range.start.line;
        jsonValue["range"]["start"]["character"] = result.range.start.column;
        jsonValue["range"]["end"]["line"] = result.range.end.line;
        jsonValue["range"]["end"]["character"] = result.range.end.column;
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("Hover", file, action);
}

void ArkServer::LocateSymbolAt(const std::string &file,
                               const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Cangjie::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        LocatedSymbol result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        bool ret = LocateSymbolAtImpl::LocateSymbolAt(*(inputAST.ast), result, pos);
        nlohmann::json jsonValue;
        if (ret) {
            jsonValue["uri"] = result.Definition.uri.file;
            jsonValue["range"]["start"]["line"] = result.Definition.range.start.line;
            jsonValue["range"]["start"]["character"] = result.Definition.range.start.column;
            jsonValue["range"]["end"]["line"] = result.Definition.range.end.line;
            jsonValue["range"]["end"]["character"] = result.Definition.range.end.column;
            if (!result.CrossMessage.empty()) {
                for (auto &message: result.CrossMessage) {
                    nlohmann::json Value;
                    Value["targetLanguage"] = message.targetLanguage;
                    Value["functionName"] = message.functionName;
                    Value["retType"] = message.retType;
                    for (auto &item: message.functionParameters) {
                        (void) Value["functionParameters"].push_back(item);
                    }
                    (void) jsonValue["crossMessage"].push_back(Value);
                }
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("Definition", file, action);
}

void ArkServer::LocateCrossSymbolAt(const CrossLanguageJumpParams &params, const Callback<ValueOrError> &reply) const
{
    CrossSymbolsResult result{};
    bool ret = CrossLanguangeDefinition::getCrossSymbols(params, result);
    nlohmann::json jsonValue;
    if (ret) {
        for (auto &iter : result.locations) {
            nlohmann::json temp;
            temp["uri"] = iter.uri.file;
            temp["range"]["start"]["line"] = iter.range.start.line;
            temp["range"]["start"]["character"] = iter.range.start.column;
            temp["range"]["end"]["line"] = iter.range.end.line;
            temp["range"]["end"]["character"] = iter.range.end.column;
            (void)jsonValue.push_back(temp);
        }
    }
    ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
    reply(value);
}

void ArkServer::FindDocumentLink(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [this, file, reply = std::move(reply)](const InputsAndAST &inputAST) {
        nlohmann::json jsonValue;
        // jsonValue should be a documentLink array
        jsonValue = nlohmann::json::array();
        ValueOrError val(ValueOrErrorCheck::VALUE, jsonValue);
        reply(val);
        std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(file);
        UpdateModifierDiag(inputAST, diagnostics);
        callback->ReadyForDiagnostics(file, inputAST.inputs.version, diagnostics);
    };
    arkScheduler->RunWithAST("DocumentLink", file, action);
}

void ArkServer::UpdateModifierDiag(const InputsAndAST &inputAST, std::vector<DiagnosticToken> &diagnostics) const
{
    if (!inputAST.ast || !inputAST.ast->file || !inputAST.ast->file->curPackage) {
        return;
    }
    auto fullPackageName = inputAST.ast->file->curPackage->fullPackageName;
    auto ret =
        CompilerCangjieProject::GetInstance()->CheckPackageModifier(*inputAST.ast->file, fullPackageName);
    if (ret) {
        auto it = std::find_if(diagnostics.begin(), diagnostics.end(), [](const DiagnosticToken &diagToken) {
            return diagToken.message ==
                   "the access level of child package can't be higher than that of parent package";
        });
        if (it != diagnostics.end()) {
            diagnostics.erase(it);
        }
    }
}

void ArkServer::FindCompletion(const CompletionParams &params, const std::string &file,
                               const Callback<ValueOrError> &reply) const
{
    Trace::Log("ArkServer::FindCompletion in.");

    auto nullValueReply = [reply]() {
        ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
        reply(value);
    };

    int fileId;
    if (Options::GetInstance().IsOptionSet("test")) {
        fileId = CompilerCangjieProject::GetInstance()->GetFileID(file);
    } else {
        fileId = CompilerCangjieProject::GetInstance()->GetFileIDForCompete(file);
    }
    if (fileId < 0) {
        nullValueReply();
        return;
    }

    Cangjie::Position pos = {
        static_cast<unsigned int>(fileId),
        params.position.line,
        params.position.column
    };

    auto action = [reply, nullValueReply, params, pos](const InputsAndAST &input) {
        CompletionResult result;
        std::string prefix;
        if (input.ast == nullptr) {
            nullValueReply();
            return;
        }
        CompletionImpl::CodeComplete(*(input.ast), pos, result, prefix);
        CompilerCangjieProject::GetInstance()->ClearParseCache();
        CompletionList completionList;
        for (auto &iter : result.completions) {
            if (prefix.back() == '.' || IsMatchingCompletion(prefix, iter.name)) {
                if (iter.name.find(BOX_DECL_PREFIX) != std::string::npos) { continue; }
                auto score = CompilerCangjieProject::GetInstance()->CalculateScore(iter, prefix, result.cursorDepth);
                completionList.items.push_back(iter.Render(GetSortText(score), prefix));
            }
        }

        nlohmann::json jsonItems;
        for (auto &iter : completionList.items) {
            nlohmann::json value;
            if (!ToJSON(iter, value)) { continue; }
            (void)jsonItems.push_back(value);
        }
        ValueOrError val(ValueOrErrorCheck::VALUE, jsonItems);
        reply(val);
        CompilerCangjieProject::GetInstance()->ClearParseCache();
    };

    arkSchedulerOfComplete->RunWithASTCache("Completion", file, pos, action);
}

void ArkServer::FindSignatureHelp(const SignatureHelpParams &params, const std::string &file,
                                  const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](InputsAndAST inputAST) {
        Cangjie::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        SignatureHelp result = SignatureHelp();
        auto nullValueReply = [reply]() {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
        };
        if (!inputAST.ast || !inputAST.ast->semaCache || !inputAST.ast->semaCache->packageInstance) {
            return;
        }
        SignatureHelpImpl signatureHelp(*(inputAST.ast), result,
                                           inputAST.ast->semaCache->packageInstance->importManager, params, pos);
        signatureHelp.FindSignatureHelp();
        if (result.signatures.empty()) {
            nullValueReply();
            return;
        }
        nlohmann::json jsonValue;
        jsonValue["activeParameter"] = result.activeParameter;
        jsonValue["activeSignature"] = result.activeSignature;
        for (auto &iter : result.signatures) {
            nlohmann::json temp;
            temp["label"] = iter.label;
            for (auto &param : iter.parameters) {
                nlohmann::json paramTemp;
                paramTemp["label"] = param;
                (void)temp["parameters"].push_back(paramTemp);
            }
            (void)jsonValue["signatures"].push_back(temp);
        }

        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    auto nullValueReply = [reply]() {
        ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
        reply(value);
    };
 
    int fileId;
    if (Options::GetInstance().IsOptionSet("test")) {
        fileId = CompilerCangjieProject::GetInstance()->GetFileID(file);
    } else {
        fileId = CompilerCangjieProject::GetInstance()->GetFileIDForCompete(file);
    }
    if (fileId < 0) {
        nullValueReply();
        return;
    }
 
    Cangjie::Position pos = {
        static_cast<unsigned int>(fileId),
        params.position.line,
        params.position.column
    };
    arkSchedulerOfSignature->RunWithASTCache("SignatureHelp", file, pos, action);
}

void ArkServer::PrepareRename(const std::string &file,
                              const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [file, params, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Cangjie::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        MessageErrorDetail errorInfo;
        Range range = PrepareRename::PrepareImpl(*(inputAST.ast), pos, errorInfo);
        if (!errorInfo.message.empty()) {
            reply(ValueOrError(ValueOrErrorCheck::ERR, errorInfo));
        } else {
            nlohmann::json jsonValue;
            if (range.start.line == -1) {
                jsonValue = nullptr;
            } else {
                jsonValue["start"]["line"] = range.start.line;
                jsonValue["start"]["character"] = range.start.column;
                jsonValue["end"]["line"] = range.end.line;
                jsonValue["end"]["character"] = range.end.column;
            }
            ValueOrError val(ValueOrErrorCheck::VALUE, jsonValue);
            reply(val);
        }
    };
    arkScheduler->RunWithAST("PrepareRename", file, action);
}

void ArkServer::Rename(const std::string &file, const RenameParams &params,
                       const Callback<ValueOrError> &reply) const
{
    auto action = [this, file, params, reply = std::move(reply)](const InputsAndAST &inputAST) mutable {
        int fileId = CompilerCangjieProject::GetInstance()->GetFileID(file);
        if (fileId < 0) {
            return;
        }
        const Cangjie::Position pos = {
            static_cast<unsigned int>(fileId),
            params.position.line,
            params.position.column
        };

        auto newName = params.newName;
        std::vector<TextDocumentEdit> result;
        std::string path = RenameImpl::Rename(*(inputAST.ast), result, pos, newName, *callback);
        nlohmann::json jsonValue;
        if (result.empty()) {
            jsonValue = nullptr;
        }
        for (auto &iter : result) {
            nlohmann::json temp;
            (void) ToJSON(iter, temp);
            (void) jsonValue.push_back(temp);
        }
        nlohmann::json ret;
        ret["documentChanges"] = jsonValue;
        ValueOrError value(ValueOrErrorCheck::VALUE, ret);
        reply(value);
        if (!path.empty()) {
            this->callback->isRenameDefined = true;
            this->callback->path = path;
        }
    };
    arkScheduler->RunWithAST("Rename", file, action);
}

void ArkServer::FindBreakpoints(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [file, reply = std::move(reply)](const InputsAndAST &inputAST) mutable {
        auto nullValueReply = [reply]() {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
        };
        if (inputAST.ast == nullptr) {
            nullValueReply();
            return;
        }
        std::set<BreakpointLocation> result;
        BreakpointsImpl::Breakpoints(*(inputAST.ast), result);
        nlohmann::json jsonValue;
        if (result.empty()) {
            jsonValue = nullptr;
        }
        for (auto &breakpointLocation : result) {
            nlohmann::json temp;
            (void) ToJSON(breakpointLocation, temp);
            (void) jsonValue.push_back(temp);
        }
        nlohmann::json ret;
        ret["breakpointLocation"] = jsonValue;
        ValueOrError value(ValueOrErrorCheck::VALUE, ret);
        reply(value);
    };
    arkScheduler->RunWithAST("FindBreakpoints", file, action);
}

void ArkServer::FindCodeLens(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [file, reply = std::move(reply)](const InputsAndAST &inputAST) mutable {
        auto nullValueReply = [reply]() {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
        };
        if (inputAST.ast == nullptr) {
            nullValueReply();
            return;
        }
        std::vector<CodeLens> result;
        CodeLensImpl::GetCodeLens(*(inputAST.ast), result);
        nlohmann::json jsonValue;
        if (result.empty()) {
            jsonValue = nullptr;
        }
        for (auto &codeLens : result) {
            nlohmann::json temp;
            (void) ToJSON(codeLens, temp);
            (void) jsonValue.push_back(temp);
        }
        nlohmann::json ret;
        ret = jsonValue;
        ValueOrError value(ValueOrErrorCheck::VALUE, ret);
        reply(value);
    };
    arkScheduler->RunWithAST("FindCodeLens", file, action);
}

void ArkServer::ChangeWatchedFiles(const std::string &file, FileChangeType type, DocCache *docMgr) const
{
    auto action = [this, file, type, docMgr](const InputsAndAST &input) {
        if (type == FileChangeType::CHANGED) {
            Logger::Instance().LogMessage(MessageType::MSG_WARNING, "enter the scenario not supported ");
            if (docMgr->GetDoc(file).version == -1) {
                Logger::Instance().LogMessage(MessageType::MSG_INFO, "creat the file:  " + file);
                const std::string &contents = GetFileContents(file);
                int64_t version = docMgr->AddDoc(file, 0, contents);
                AddDoc(file, contents, version, ark::NeedDiagnostics::YES, true);
            }
            return;
        }
        if (type == FileChangeType::CREATED) {
            Logger::Instance().LogMessage(MessageType::MSG_INFO, "creat the file:  " + file);
            const std::string &contents = GetFileContents(file);
            int64_t version = docMgr->AddDoc(file, 0, contents);
            AddDoc(file, contents, version, ark::NeedDiagnostics::YES, true);
            return;
        }
        if (type == FileChangeType::DELETED) {
            Logger::Instance().LogMessage(MessageType::MSG_INFO, "delete the file:  " + file);
            CompilerCangjieProject::GetInstance()->IncrementForFileDelete(file);
            CompilerCangjieProject::GetInstance()->GetBgIndexDB()->DeleteFiles({file});
            this->callback->RemoveDocByFile(input.inputs.fileName);
        }
        if (!FileUtil::FileExist(input.onEditFile)) {
            return;
        }
        std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(input.onEditFile);
        callback->ReadyForDiagnostics(input.onEditFile, input.inputs.version, diagnostics);
    };

    auto taskName = "ChangeWatchedFiles " + file;
    arkScheduler->RunWithAST(taskName, file, action);
}

void ArkServer::AddDoc(const std::string &file, const std::string &contents,
    int64_t version, NeedDiagnostics needDiagnostics, bool forceRebuild) const
{
    ParseInputs parseInputs;
    parseInputs.fileName = file;
    parseInputs.contents = contents;
    parseInputs.version = version;
    parseInputs.forceRebuild = forceRebuild;
    arkScheduler->Update(parseInputs, needDiagnostics);
}

void ArkServer::FindDocumentSymbol(const DocumentSymbolParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply](const InputsAndAST &inputAST) mutable {
        std::vector<DocumentSymbol> result;
        auto filePath = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
        // To avoid queue task in runWithAST crashing
        if (inputAST.ast == nullptr || !CompilerCangjieProject::GetInstance()->FileHasSemaCache(filePath)
            || Cangjie::FileUtil::HasExtension(filePath, CONSTANTS::CANGJIE_MACRO_FILE_EXTENSION)) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        DocumentSymbolImpl::FindDocumentSymbols(*(inputAST.ast), result);
        nlohmann::json jsonValue;
        for (auto &documentSymbol: result) {
            nlohmann::json temp;
            if (!ToJSON(documentSymbol, temp)) {
                continue;
            }
            (void)jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    arkScheduler->RunWithAST("DocumentSymbol", file, action);
}

void ArkServer::FindOverrideMethods(const std::string &file,
                               const OverrideMethodsParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Cangjie::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        FindOverrideMethodResult result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        FindOverrideMethodsImpl::FindOverrideMethods(*(inputAST.ast), result, pos, params.isExtend);
        nlohmann::json jsonValue;
        for (auto item: result.overrideMethods) {
            nlohmann::json overrideItem;
            overrideItem["importItem"] = item.package + CONSTANTS::DOT + item.identifier;
            overrideItem["fullPackageName"] = item.package;
            overrideItem["identifier"] = item.identifier;
            overrideItem["kind"] = item.kind;
            for (auto method: item.overrideMethodInfos) {
                nlohmann::json jsonItem;
                jsonItem["deprecated"] = method.deprecated;
                jsonItem["isProp"] = method.isProp;
                jsonItem["signatureWithRet"] = method.signatureWithRet;
                jsonItem["insertText"] = method.insertText;
                overrideItem["data"].push_back(jsonItem);
            }
            jsonValue.push_back(overrideItem);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("OverrideMethods", file, action);
}

Cangjie::Position ArkServer::AlterPosition(const std::string &file,
                                           const TextDocumentPositionParams &params) const
{
    int fileId = CompilerCangjieProject::GetInstance()->GetFileID(file);
    if (fileId < 0) {
        return INVALID_POSITION;
    }
    return Cangjie::Position{
            static_cast<unsigned int>(fileId),
            params.position.line,
            params.position.column
    };
}

static std::vector<std::unique_ptr<Tweak::Selection>> CreateTweakSelection(const InputsAndAST &inputAST,
    const std::string &file, Range range, std::map<std::string, std::string> extraOptions)
{
    std::vector<std::unique_ptr<Tweak::Selection>> result;

    SelectionTree::CreateEach(*inputAST.ast, file, range.start,
        range.end, [&](SelectionTree selectionTree) {
            // cannot tweak:
            // if selected range is not GLOBAL_VAR/MEMBER_VAR/GLOBAL_FUNC_BODY/MEMBER_FUNC_BODY/EXTEND_FUNC_BODY
            if (!selectionTree.root() || selectionTree.SelectedScope() == SelectionTree::Scope::UNKNOWN) {
                return false;
            }
            auto tweakSelection =
                std::make_unique<Tweak::Selection>(*inputAST.ast, range, std::move(selectionTree), extraOptions);

            result.push_back(std::move(tweakSelection));
            return true;
        });
    return std::move(result);
}

void ArkServer::EnumerateTweaks(const std::string &file, Range range, const Callback<std::vector<TweakRef>> &cb) const
{
    auto action = [file, range, cb = std::move(cb)]
        (const InputsAndAST &inputAST) mutable {
            std::vector<TweakRef> res;
            if (!inputAST.ast) {
                cb(std::move(res));
                return;
            }

            // update pos fileID
            range.start.fileID = inputAST.ast->fileID;
            range.end.fileID = inputAST.ast->fileID;

            // check current token is the kind which required in function CheckTokenKind(TokenKind)
            range.start = PosFromIDE2Char(range.start);
            PositionIDEToUTF8(inputAST.ast->tokens, range.start, *inputAST.ast->file);
            range.end = PosFromIDE2Char(range.end);
            PositionIDEToUTF8(inputAST.ast->tokens, range.end, *inputAST.ast->file);

            // get Selections by range
            // visit all Tweaks check tweak available
            auto selections =
                CreateTweakSelection(inputAST, file, range, std::map<std::string, std::string>());
            if (selections.empty()) {
                cb(std::move(res));
                return;
            }
            std::unordered_set<std::string> preparedTweaks;
            auto deduplicatingFilter = [&preparedTweaks](const Tweak &tweak) {
                return !preparedTweaks.count(tweak.Id());
            };
            for (const auto &selection : selections) {
                if (!selection) {
                    continue;
                }
                for (const auto &tweak
                    : Tweak::PrepareTweaks(*selection, deduplicatingFilter)) {
                    if (!tweak) {
                        continue;
                    }
                    res.push_back({tweak->Id(), tweak->Title(), tweak->Kind(),
                        tweak->ExtraOptions()});
                    preparedTweaks.insert(tweak->Id());
                }
            }
            cb(std::move(res));
        };
    arkScheduler-> RunWithAST("EnumerateTweaks", file, std::move(action));
}

void ArkServer::ApplyTweak(const std::string &file, Range selection, const std::string &id,
    std::map<std::string, std::string> extraOptions, const Callback<Tweak::Effect> &cb)
{
    auto action = [file, selection, id, extraOptions, cb = std::move(cb), this]
        (const InputsAndAST &inputAST) mutable {
            Tweak::Effect effect;
            if (!inputAST.ast || !inputAST.ast->sourceManager) {
                return cb(std::move(effect));
            }

            selection.start.fileID = inputAST.ast->fileID;
            selection.end.fileID = inputAST.ast->fileID;

            selection.start = PosFromIDE2Char(selection.start);
            PositionIDEToUTF8(inputAST.ast->tokens, selection.start, *inputAST.ast->file);
            selection.end = PosFromIDE2Char(selection.end);
            PositionIDEToUTF8(inputAST.ast->tokens, selection.end, *inputAST.ast->file);

            auto selections = CreateTweakSelection(inputAST, file, selection, extraOptions);
            if (selections.empty()) {
                return cb(std::move(effect));
            }
            for (const auto &sel : selections) {
                if (!sel) {
                    continue;
                }
                auto tweak = Tweak::PrepareTweak(id, *sel);
                if (!tweak.has_value() || !tweak.value()) {
                    continue;
                }
                auto effectOpt = tweak.value()->Apply(*sel);
                if (effectOpt.has_value()) {
                    effect = effectOpt.value();
                    break;
                }
            }

            return cb(std::move(effect));
        };
    arkScheduler->RunWithAST("ApplyTweak", file, std::move(action));
}
} // namespace ark
