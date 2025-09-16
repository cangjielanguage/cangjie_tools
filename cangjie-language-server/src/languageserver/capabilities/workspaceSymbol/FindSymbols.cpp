// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "FindSymbols.h"
#include "../../CompilerCangjieProject.h"

namespace ark {
namespace lsp {

std::vector<SymbolInformation> GetWorkspaceSymbols(const std::string &query)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "GetWorkspaceSymbols in.");
    auto index = ark::CompilerCangjieProject::GetInstance()->GetIndex();
    std::vector<SymbolInformation> result;
    if (!index || query.empty()) {
        return result;
    }
    FuzzyFindRequest req;
    req.query = query;
    index->FuzzyFind(req, [&result](const Symbol &sym) {
        if (Options::GetInstance().IsOptionSet("test") && sym.isCjoSym) {
            return;
        }
        if (sym.kind == ASTKind::FUNC_PARAM) {
            return;
        }
        std::string realSig = sym.signature;
        if (sym.kind == ASTKind::FUNC_DECL) {
            auto lp = sym.signature.find_first_of('(');
            auto la = sym.signature.find_first_of('<');
            std::string funcName = sym.signature.substr(0, std::min(lp, la));
            if (funcName == "init") {
                realSig = sym.name + realSig.substr(std::min(lp, la));
            }
        }
        Location loc;
        loc.uri.file = URI::URIFromAbsolutePath(sym.location.fileUri).ToString();
        loc.range = TransformFromChar2IDE({sym.location.begin, sym.location.end});
        std::string containerName = sym.scope;
        std::replace(containerName.begin(), containerName.end(), ':', '.');
        SymbolInformation info{.name = realSig,
                               .kind = ark::GetSymbolKind(sym.kind),
                               .location = loc,
                               .containerName = containerName};
        result.push_back(info);
    });
    return result;
}

} // namespace lsp
} // namespace ark