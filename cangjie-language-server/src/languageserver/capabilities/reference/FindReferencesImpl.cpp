// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "FindReferencesImpl.h"
#include "../../CompilerCangjieProject.h"
#include "../../index/Symbol.h"

using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Cangjie::Meta;

namespace ark {

std::string FindReferencesImpl::curFilePath = "";

void FindReferencesImpl::GetCurPkgUesage(Ptr<Decl> decl, const ArkAST &ast, ReferencesResult &result)
{
    if (!decl || !ast.file || !ast.file->curPackage) {
        return;
    }
    auto user = FindDeclUsage(*decl, *ast.file->curPackage);
    for (const auto &U : user) {
        if (U->astKind == ASTKind::MEMBER_ACCESS) {
            continue;
        }
        auto range = GetProperRange(U, ast.tokens);
        Location loc = {URI::URIFromAbsolutePath(U->curFile->filePath).ToString(), range};
        (void)result.References.emplace(loc);
    }
}

void FindReferencesImpl::FindReferences(const ArkAST &ast, ReferencesResult &result, Position pos)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "FindReferencesImpl::FindReferences in.");

    // adjust position from IDE to AST
    pos = PosFromIDE2Char(pos);
    PositionIDEToUTF8(ast.tokens, pos, *ast.file);

    if (ast.IsFilterToken(pos)) {
        return;
    }
    // get curFilePath
    curFilePath = ast.file ? ast.file->filePath : "";
    pos.fileID = ast.fileID;
    std::vector<Symbol *> syms;
    std::vector<Ptr<Cangjie::AST::Decl> > decls;
    Ptr<Decl> oldDecl = ast.GetDeclByPosition(pos, syms, decls, {true, true});
    if (oldDecl == nullptr) {
        return;
    }
    if (syms[0] && IsResourcePos(ast, syms[0]->node, pos)) {
        return;
    }
    DealMemberParam(curFilePath, syms, decls, oldDecl);
    // generic param decl
    if (oldDecl->astKind == ASTKind::GENERIC_PARAM_DECL) {
        DealGenericParamDecl(ast, result, oldDecl, syms);
        return;
    }

    if (!decls.empty()) {
        // First verify if the downstream package status is stale
        auto definedPkg = decls[0]->fullPackageName;
        // Find all downstream packages
        auto downPackages = CompilerCangjieProject::GetInstance()->GetDependencyGraph()->GetDependents(definedPkg);
        // Check the status of all downstream packages
        auto tasks = CompilerCangjieProject::GetInstance()->GetCjoManager()->CheckStatus(downPackages);
        // Compile all downstream packages before searching for references
        CompilerCangjieProject::GetInstance()->SubmitTasksToPool(tasks);
    }

    lsp::SymbolIndex *index = ark::CompilerCangjieProject::GetInstance()->GetIndex();
    if (!index) {
        return;
    }
    int curIdx = ast.GetCurToken(pos, 0, static_cast<int>(ast.tokens.size()) - 1);
    for (auto &decl : decls) {
        if (decl->astKind == Cangjie::AST::ASTKind::PACKAGE_DECL) { return; }
        if (!IsGlobalOrMemberOrItsParam(*decl)) {
            // For a local variable, maybe a function or a variable
            GetCurPkgUesage(decl, ast, result);
            continue;
        }

        // find reference by memIndex
        auto id = GetSymbolId(*decl);
        if (id == lsp::INVALID_SYMBOL_ID) {
            continue;
        }

        std::unordered_set<lsp::SymbolID> ids;
        lsp::SymbolID topId = id;
        index->FindRiddenUp(id, ids, topId);
        index->FindRiddenDown(topId, ids);
        (void)ids.insert(id);

        lsp::RefsRequest req{ids, lsp::RefKind::REFERENCE};
        lsp::Ref definition{ {}, {}, 0 };
        index->RefsFindReference(req, definition, [&result, pos, curIdx, ast](const lsp::Ref &ref) {
            if (ref.isCjoRef) {
                return;
            }
            if (ref.location.IsZeroLoc()) {
                return;
            }
            auto realPath = ref.location.fileUri;
            if (EndsWith(realPath, ".macrocall")) {
                return;
            }
            if (IsInvalidRef(ref, pos, curIdx, ast)) {
                return;
            }
            CompilerCangjieProject::GetInstance()->GetRealPath(realPath);
            Location loc{URI::URIFromAbsolutePath(realPath).ToString(),
                         TransformFromChar2IDE({ref.location.begin, ref.location.end})};
            (void)result.References.emplace(loc);
        });
        if (definition.container != 0) {
            auto realPath = definition.location.fileUri;
            CompilerCangjieProject::GetInstance()->GetRealPath(realPath);
            Location defLoc{URI::URIFromAbsolutePath(realPath).ToString(),
                TransformFromChar2IDE({definition.location.begin, definition.location.end})};
            auto it = result.References.find(defLoc);
            if (it != result.References.end()) {
                result.References.erase(it);
            }
        }
    }
}

void FindReferencesImpl::FindFileReferences(const ArkAST &ast, ReferencesResult &result)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "FindReferencesImpl::FindFileReferences in.");
    for (auto& decl: ast.file->decls) {
        FindReferences(ast, result, decl->GetIdentifierPos() + DEFAULT_POSITION);
    }
}

bool FindReferencesImpl::IsInvalidRef(const lsp::Ref& ref, Position pos, int curIdx, const ArkAST &ast)
{
    if (curIdx == -1) {
        return false;
    }
    Token posToken = ast.tokens[static_cast<unsigned int>(curIdx)];
    if (pos.fileID == ref.location.begin.fileID && ref.location.begin.column < ref.location.end.column - 1) {
        Position refPos = {ref.location.begin.fileID, ref.location.begin.line, ref.location.begin.column + 1};
        int refIdx = ast.GetCurToken(refPos, 0, static_cast<int>(ast.tokens.size()) - 1);
        if (refIdx == -1) {
            return false;
        }
        Token refToken = ast.tokens[static_cast<unsigned int>(refIdx)];
        if (refToken.Value() != posToken.Value() && refToken.Value() == "@") {
            return true;
        }
    }
    return false;
}

void FindReferencesImpl::DealGenericParamDecl(
    const ArkAST &ast, ReferencesResult &result, Ptr<Decl> oldDecl, std::vector<Symbol *> &syms)
{
    if (auto temp = ast.FindRealGenericParamDeclForExtend(oldDecl->identifier, syms)) {
        oldDecl = temp;
    }
    GetCurPkgUesage(oldDecl, ast, result);
    auto inheritable = dynamic_cast<InheritableDecl*>(oldDecl->outerDecl.get());
    if (!inheritable) {
        return;
    }
    std::string pkgName = GetPkgNameFromNode(ast.file);
    auto extendDecls = CompilerCangjieProject::GetInstance()->GetExtendDecls(inheritable, pkgName);
    for (auto &extendDecl : extendDecls) {
        if (!extendDecl || !extendDecl->generic) {
            continue;
        }
        for (auto &genericDecl : extendDecl->generic->typeParameters) {
            if (genericDecl && genericDecl->identifier == oldDecl->identifier) {
                GetCurPkgUesage(genericDecl.get(), ast, result);
            }
        }
    }
}
} // namespace ark
