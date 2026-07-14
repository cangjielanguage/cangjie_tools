// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "MemIndex.h"
#include "../CompilerCangjieProject.h"
#include "Symbol.h"
#include <unordered_set>

namespace ark {
namespace lsp {
PackageRelation GetPackageRelation(const std::string &srcFullPackageName,
                                   const std::string &targetFullPackageName)
{
    if (srcFullPackageName.length() < targetFullPackageName.length()
        && targetFullPackageName.find(srcFullPackageName) == 0
        && targetFullPackageName[srcFullPackageName.length()] == '.') {
        return PackageRelation::PARENT;
    }
    if (srcFullPackageName.length() > targetFullPackageName.length()
        && srcFullPackageName.find(targetFullPackageName) == 0
        && srcFullPackageName[targetFullPackageName.length()] == '.') {
        return PackageRelation::CHILD;
    }
    auto srcRoot = Cangjie::Utils::SplitQualifiedName(srcFullPackageName).front();
    auto targetRoot = Cangjie::Utils::SplitQualifiedName(targetFullPackageName).front();
    return srcRoot == targetRoot ? PackageRelation::SAME_MODULE : PackageRelation::NONE;
}

void MemIndex::FuzzyFind(const FuzzyFindRequest &req, std::function<void(const Symbol &)> callback) const
{
    for (const auto &pkgSyms : pkgSymsMap) {
        for (const auto &sym : pkgSyms.second) {
            if (!IsMatchingCompletion(req.query, sym.name)) {
                continue;
            }
            callback(sym);
        }
    }
}

void MemIndex::Lookup(const LookupRequest &req, std::function<void(const Symbol &)> callback) const
{
    for (const auto &id : req.ids) {
        for (const auto &pkgSyms : pkgSymsMap) {
            for (const auto &sym : pkgSyms.second) {
                if (id == sym.id) {
                    callback(sym);
                }
            }
        }
    }
}
// LCOV_EXCL_START
void MemIndex::FindPkgSyms(const PkgSymsRequest &req, std::function<void(const Symbol &)> callback) const
{
    auto it = pkgSymsMap.find(req.fullPkgName);
    if(it == pkgSymsMap.end()) {
        return;
    }
    for(const auto &sym : it->second) {
        callback(sym);
    }
}
// LCOV_EXCL_STOP
void MemIndex::Refs(const RefsRequest &req, std::function<void(const Ref &)> callback) const
{
    for (const auto &id : req.ids) {
        for (const auto &pkgRefs : pkgRefsMap) {
            auto symRef = pkgRefs.second.find(id);
            if (symRef == pkgRefs.second.end()) {
                continue;
            }
            for (const auto &sym : symRef->second) {
                if (!static_cast<int>(req.filter & sym.kind)) {
                    continue;
                }
                if (!sym.isSuper) {
                    callback(sym);
                }
            }
        }
    }
}
// LCOV_EXCL_START
void MemIndex::FileRefs(const FileRefsRequest &req,
    std::function<void(const Ref &ref, const SymbolID symId)> callback) const
{
    auto it = pkgRefsMap.find(req.fullPkgName);
    if (it == pkgRefsMap.end())
    {
        return;
    }
    auto pkgRefs = it->second;
    for (const auto &refs : pkgRefs) {
        for (const auto &ref : refs.second) {
            if (!static_cast<int>(req.filter & ref.kind) || ref.location.begin.fileID != req.fileID) {
                continue;
            }
            callback(ref, refs.first);
        }
    }
}
// LCOV_EXCL_STOP
void MemIndex::RefsFindReference(const RefsRequest &req,
    Ref &definition, std::function<void(const Ref &)> callback) const
{
    for (const auto &id : req.ids) {
        for (const auto &pkgRefs : pkgRefsMap) {
            auto symRef = pkgRefs.second.find(id);
            if (symRef == pkgRefs.second.end()) {
                continue;
            }
            for (const auto &sym : symRef->second) {
                if (sym.kind == RefKind::DEFINITION) {
                    definition = sym;
                }
                if (!static_cast<int>(req.filter & sym.kind)) {
                    continue;
                }
                callback(sym);
            }
        }
    }
}

void MemIndex::Callees(const std::string &pkgName, const SymbolID &declId,
    std::function<void(const SymbolID &, const Ref &)> callback) const
{
    auto pkgSymRefs = pkgRefsMap.find(pkgName);
    if (pkgSymRefs == pkgRefsMap.end()) {
        return;
    }
    for (const auto& symbolRef : pkgSymRefs->second) {
        auto declSymId = symbolRef.first;
        for (const auto& ref : symbolRef.second) {
            if (ref.container != declId) {
                continue;
            }
            callback(declSymId, ref);
        }
    }
}
// LCOV_EXCL_START
void MemIndex::Relations(const RelationsRequest &req, std::function<void(const Relation &)> callback) const
{
    for (const auto &pkgRelations : pkgRelationsMap) {
        for (const auto &spo : pkgRelations.second) {
            if (spo.subject == req.id && spo.predicate == req.predicate) {
                callback(spo);
            }
            if (spo.object == req.id && spo.predicate == req.predicate) {
                callback(spo);
            }
        }
    }
}
// LCOV_EXCL_STOP
Symbol MemIndex::GetAimSymbol(const Decl& decl)
{
    auto pkgName = CompilerCangjieProject::GetInstance()->GetFinalDownStreamFullPkgName(decl.fullPackageName);
    auto symbolID = GetSymbolId(decl);
    if (pkgSymsMap.find(pkgName) == pkgSymsMap.end()) { return {}; }
    for (auto &sym: pkgSymsMap[pkgName]) {
        if (sym.id == symbolID) {
            return sym;
        }
    }
    return {};
}
// LCOV_EXCL_START
void MemIndex::FindImportSymsOnCompletion(
    const std::pair<std::unordered_set<SymbolID>, std::unordered_set<SymbolID>>& filterSyms,
    const SymbolSearchContext &context, const std::string &prefix,
    std::function<void(const std::string &, const Symbol &, const CompletionItem &)> callback)
{
    (void)prefix;
    if (Options::GetInstance().IsOptionSet("test")) {
        return;
    }
    const auto &curPkgName = context.curPkgName;
    const auto &curModule = context.curModule;
    const auto &normalCompleteSyms  = filterSyms.first;
    const auto &importDeclSyms  = filterSyms.second;
    size_t normalCompleteCount = 0;
    size_t importDeclCount = 0;
    std::unordered_set<std::string> curModuleDeps =
        CompilerCangjieProject::GetInstance()->GetOneModuleDirectDeps(curModule, context.includeScriptRequire);
    for (const auto &pkgSyms : pkgSymsMap) {
        // filter curPackage sym
        if (curPkgName == pkgSyms.first ||
            !CompilerCangjieProject::GetInstance()->IsVisibleForPackage(curPkgName, pkgSyms.first)) {
            continue;
        }
        auto relation = GetPackageRelation(curPkgName, pkgSyms.first);
        for (const auto &sym : pkgSyms.second) {
            // filter symbols that not in correct package
            if (sym.scope.find(pkgSyms.first) == std::string::npos) {
                continue;
            }
            bool isPkgAccess =
                sym.pkgModifier == Modifier::PUBLIC
                || (relation == PackageRelation::CHILD && (sym.pkgModifier == Modifier::INTERNAL
                                                              || sym.pkgModifier == Modifier::PROTECTED))
                || (relation == PackageRelation::SAME_MODULE && sym.pkgModifier == Modifier::PROTECTED)
                || (relation == PackageRelation::PARENT && sym.pkgModifier == Modifier::PROTECTED);
            if (!isPkgAccess) {
                break;
            }
            // filter symbols that not dependent by curModule
            if (!sym.isCjoSym && !curModuleDeps.count(sym.curModule)) {
                continue;
            }
            // filter symbols that are already in the NormalComplete
            if (sym.id != 0 && normalCompleteCount < normalCompleteSyms.size() && normalCompleteSyms.count(sym.id)) {
                normalCompleteCount++;
                continue;
            }
            // filter imported syms
            if (importDeclCount < importDeclSyms.size() && importDeclSyms.count(sym.id)) {
                importDeclCount++;
                continue;
            }
            // filter not top decl
            if (sym.scope.find(':') != std::string::npos && sym.scope != pkgSyms.first) {
                continue;
            }
            // filter by modifier
            bool isAccessible =
                sym.modifier == ark::lsp::Modifier::PUBLIC || (relation == PackageRelation::CHILD &&
                    (sym.modifier == ark::lsp::Modifier::INTERNAL || sym.modifier == ark::lsp::Modifier::PROTECTED)) ||
                    (relation == PackageRelation::SAME_MODULE && sym.modifier == ark::lsp::Modifier::PROTECTED) ||
                    (relation == PackageRelation::PARENT && sym.modifier == ark::lsp::Modifier::PROTECTED);
            // filter root pkg symbols if cur module is combined
            if (isAccessible
                && !CompilerCangjieProject::GetInstance()->IsCombinedSym(curModule, curPkgName, pkgSyms.first)) {
                for (const auto &completionItem : sym.completionItems) {
                    callback(pkgSyms.first, sym, completionItem);
                }
            }
        }
    }
}

void MemIndex::FindExtendSymsOnCompletion(const SymbolID &dotCompleteSym,
    const std::unordered_set<SymbolID> &visibleMembers,
    const SymbolSearchContext &context,
    const std::function<void(const std::string &, const std::string &,
        const Symbol &, const CompletionItem &)>& callback)
{
    if (Options::GetInstance().IsOptionSet("test")) {
        return;
    }
    const auto &curPkgName = context.curPkgName;
    const auto &curModule = context.curModule;
    std::unordered_set<std::string> curModuleDeps =
        CompilerCangjieProject::GetInstance()->GetOneModuleDirectDeps(curModule, context.includeScriptRequire);
    for (const auto &extendSyms : pkgExtendsMap) {
        // filter curPackage sym
        std::string pkgName = extendSyms.first;
        if (curPkgName == pkgName ||
            !CompilerCangjieProject::GetInstance()->IsVisibleForPackage(curPkgName, pkgName)) {
            continue;
        }
        auto relation = GetPackageRelation(curPkgName, pkgName);
        auto syms = pkgSymsMap[pkgName];
        std::unordered_map<SymbolID, Symbol*> symMap;
        for (auto& sym : syms) {
            symMap.insert_or_assign(sym.id, &sym);
        }
        auto checkAccessible = [&relation](const Modifier& modifier) -> bool {
            bool isAccessible =
                modifier == Modifier::PUBLIC || (relation == PackageRelation::CHILD &&
                    (modifier == Modifier::INTERNAL || modifier == Modifier::PROTECTED)) ||
                    (relation == PackageRelation::SAME_MODULE && modifier == Modifier::PROTECTED) ||
                    (relation == PackageRelation::PARENT && modifier == Modifier::PROTECTED);
            return isAccessible;
        };
        for (const auto &extendSlab : extendSyms.second) {
            if (extendSlab.first == dotCompleteSym) {
                for (const auto& symbol : extendSlab.second) {
                    if (symbol.id == INVALID_SYMBOL_ID || visibleMembers.find(symbol.id) != visibleMembers.end()) {
                        continue;
                    }
                    auto* sym = symMap[symbol.id];
                    // filter symbols that not dependent by curModule
                    if (!sym || (!sym->isCjoSym && !curModuleDeps.count(sym->curModule))) {
                        continue;
                    }
                    // filter by modifier
                    if (checkAccessible(symbol.modifier) && checkAccessible(sym->modifier)) {
                        for (const auto &completionItem : sym->completionItems) {
                            callback(pkgName, symbol.interfaceName, *sym, completionItem);
                        }
                    }
                }
                break;
            }
        }
    }
}

void MemIndex::FindExtendSymsOnCompletionBatch(
    const std::unordered_set<SymbolID> &ids,
    const std::unordered_set<SymbolID> &allVisibleMembers,
    const SymbolSearchContext &context, bool filterStatic,
    const std::function<void(const std::string &, const std::string &,
        const Symbol &, const CompletionItem &)>& callback)
{
    if (ids.empty()) {
        return;
    }
    const auto &curPkgName = context.curPkgName;
    const auto &curModule = context.curModule;
    std::unordered_set<std::string> curModuleDeps =
        CompilerCangjieProject::GetInstance()->GetOneModuleDirectDeps(curModule, context.includeScriptRequire);

    for (const auto &extendSyms : pkgExtendsMap) {
        const std::string &pkgName = extendSyms.first;
        if (curPkgName == pkgName ||
                !CompilerCangjieProject::GetInstance()->IsVisibleForPackage(curPkgName, pkgName)) {
            continue;
        }

        auto relation = GetPackageRelation(curPkgName, pkgName);
        auto syms = pkgSymsMap[pkgName];
        std::unordered_map<SymbolID, Symbol*> symMap;
        for (auto& sym : syms) {
            symMap.insert_or_assign(sym.id, &sym);
        }

        auto checkAccessible = [&relation](const Modifier& modifier) -> bool {
            bool isAccessible =
                modifier == Modifier::PUBLIC || (relation == PackageRelation::CHILD &&
                    (modifier == Modifier::INTERNAL || modifier == Modifier::PROTECTED)) ||
                    (relation == PackageRelation::SAME_MODULE && modifier == Modifier::PROTECTED) ||
                    (relation == PackageRelation::PARENT && modifier == Modifier::PROTECTED);
            return isAccessible;
        };

        auto handleExtendItem = [&] (const std::vector<ExtendItem> &extendItem) {
            for (const auto& symbol : extendItem) {
                if (symbol.id == INVALID_SYMBOL_ID || symbol.isStatic != filterStatic ||
                    allVisibleMembers.find(symbol.id) != allVisibleMembers.end()) {
                    continue;
                }
                auto* sym = symMap[symbol.id];
                if (!sym || (!sym->isCjoSym && !curModuleDeps.count(sym->curModule))) {
                    continue;
                }
                if (!checkAccessible(symbol.modifier) || !checkAccessible(sym->modifier)) {
                    continue;
                }
                for (const auto &completionItem : sym->completionItems) {
                    callback(pkgName, symbol.interfaceName, *sym, completionItem);
                }
            }
        };

        for (const auto &extendSlab : extendSyms.second) {
            if (ids.find(extendSlab.first) == ids.end()) {
                continue;
            }
            handleExtendItem(extendSlab.second);
        }
    }
}

void MemIndex::FindImportReExportSymsOnCompletion(
    const std::pair<std::unordered_set<SymbolID>, std::unordered_set<SymbolID>>& filterSyms,
    const std::string &curPkgName, const std::string &curModule, const std::string &prefix,
    std::function<void(const std::string &, const ReExportSymbol &, const CompletionItem &)> callback)
{
    (void)prefix;
    const auto &normalCompleteSyms  = filterSyms.first;
    const auto &importDeclSyms  = filterSyms.second;
    std::unordered_set<std::string> curModuleDeps =
        CompilerCangjieProject::GetInstance()->GetOneModuleDirectDeps(curModule);
    for (const auto &pkgReExportSymbols: pkgReExportSymsMap) {
        if (pkgReExportSymbols.first == curPkgName ||
            !CompilerCangjieProject::GetInstance()->IsVisibleForPackage(curPkgName, pkgReExportSymbols.first) ||
            CompilerCangjieProject::GetInstance()->IsCombinedSym(curModule, curPkgName, pkgReExportSymbols.first)) {
            continue;
        }
        auto relation = GetPackageRelation(curPkgName, pkgReExportSymbols.first);
        // module name for dependency check
        std::string symModule = CompilerCangjieProject::GetInstance()->GetModuleNameByFile(
            "", pkgReExportSymbols.first);
        for (const auto &sym: pkgReExportSymbols.second) {
            // filter symbols that not dependent by curModule
            if (!sym.isCjoSym && !curModuleDeps.count(symModule)) {
                continue;
            }
            bool isAccessiable =
                sym.modifier == Modifier::PUBLIC || (relation == PackageRelation::CHILD &&
                    (sym.modifier == Modifier::INTERNAL || sym.modifier == Modifier::PROTECTED)) ||
                    (relation == PackageRelation::SAME_MODULE && sym.modifier == Modifier::PROTECTED) ||
                    (relation == PackageRelation::PARENT && sym.modifier == Modifier::PROTECTED);
            if (!isAccessiable || sym.id == INVALID_SYMBOL_ID) {
                continue;
            }
            if (normalCompleteSyms.count(sym.id)) {
                continue;
            }
            if (importDeclSyms.count(sym.id)) {
                continue;
            }
            for (const auto &completionItem : sym.completionItems) {
                callback(pkgReExportSymbols.first, sym, completionItem);
            }
        }
    }
}

void MemIndex::FindImportReExportSymsOnQuickFix(const SymbolSearchContext &context,
    const std::unordered_set<SymbolID> &importDeclSyms,
    const std::string &identifier,
    const std::function<void(const std::string &, const ReExportSymbol &)>& callback)
{
    const auto &curPkgName = context.curPkgName;
    const auto &curModule = context.curModule;
    std::unordered_set<std::string> curModuleDeps =
        CompilerCangjieProject::GetInstance()->GetOneModuleDirectDeps(curModule, context.includeScriptRequire);
    for (const auto &pkgReExportSymbols: pkgReExportSymsMap) {
        if (pkgReExportSymbols.first == curPkgName ||
            !CompilerCangjieProject::GetInstance()->IsVisibleForPackage(curPkgName, pkgReExportSymbols.first) ||
            CompilerCangjieProject::GetInstance()->IsCombinedSym(curModule, curPkgName, pkgReExportSymbols.first)) {
            continue;
        }
        // module name for dependency check
        std::string symModule = CompilerCangjieProject::GetInstance()->GetModuleNameByFile(
            "", pkgReExportSymbols.first);
        auto relation = GetPackageRelation(curPkgName, pkgReExportSymbols.first);
        for (const auto &sym: pkgReExportSymbols.second) {
            // filter diff name - exact match
            if (sym.name != identifier) {
                continue;
            }
            // filter symbols that not dependent by curModule
            if (!sym.isCjoSym && !curModuleDeps.count(symModule)) {
                continue;
            }
            bool isAccessible =
                sym.modifier == Modifier::PUBLIC || (relation == PackageRelation::CHILD &&
                    (sym.modifier == Modifier::INTERNAL || sym.modifier == Modifier::PROTECTED)) ||
                    (relation == PackageRelation::SAME_MODULE && sym.modifier == Modifier::PROTECTED) ||
                    (relation == PackageRelation::PARENT && sym.modifier == Modifier::PROTECTED);
            if (!isAccessible || sym.id == INVALID_SYMBOL_ID) {
                continue;
            }
            // filter already imported syms
            if (importDeclSyms.count(sym.id)) {
                continue;
            }
            callback(pkgReExportSymbols.first, sym);
        }
    }
}

void MemIndex::FindImportSymsOnQuickFix(const SymbolSearchContext &context,
    const std::unordered_set<SymbolID> &importDeclSyms,
    const std::string& identifier,
    const std::function<void(const std::string &, const Symbol &)>& callback)
{
    const auto &curPkgName = context.curPkgName;
    const auto &curModule = context.curModule;
    size_t importDeclCount = 0;
    std::unordered_set<std::string> curModuleDeps =
        CompilerCangjieProject::GetInstance()->GetOneModuleDirectDeps(curModule, context.includeScriptRequire);
    for (const auto &pkgSyms : pkgSymsMap) {
        // filter curPackage sym
        if (curPkgName == pkgSyms.first) {
            continue;
        }
        auto relation = GetPackageRelation(curPkgName, pkgSyms.first);
        for (const auto &sym : pkgSyms.second) {
            bool isPkgAccess =
                sym.pkgModifier == Modifier::PUBLIC || (relation == PackageRelation::CHILD &&
                    (sym.pkgModifier == Modifier::INTERNAL || sym.pkgModifier == Modifier::PROTECTED)) ||
                    (relation == PackageRelation::SAME_MODULE && sym.pkgModifier == Modifier::PROTECTED) ||
                    (relation == PackageRelation::PARENT && sym.pkgModifier == Modifier::PROTECTED);
            if (!isPkgAccess) {
                break;
            }
            // filter diff name
            if (sym.name != identifier) {
                continue;
            }
            // filter symbols that not dependent by curModule
            if (!sym.isCjoSym && !curModuleDeps.count(sym.curModule)) {
                continue;
            }
            // filter imported syms
            if (importDeclCount < importDeclSyms.size() && importDeclSyms.count(sym.id)) {
                importDeclCount++;
                continue;
            }
            // filter not top decl
            if (sym.scope.find(':') != std::string::npos && sym.scope != pkgSyms.first) {
                continue;
            }
            // filter by modifier
            bool isAccessible =
                sym.modifier == ark::lsp::Modifier::PUBLIC || (relation == PackageRelation::CHILD &&
                    (sym.modifier == ark::lsp::Modifier::INTERNAL || sym.modifier == ark::lsp::Modifier::PROTECTED)) ||
                    (relation == PackageRelation::SAME_MODULE && sym.modifier == ark::lsp::Modifier::PROTECTED) ||
                    (relation == PackageRelation::PARENT && sym.modifier == ark::lsp::Modifier::PROTECTED);
            // filter root pkg symbols if cur module is combined
            if (isAccessible
                && !CompilerCangjieProject::GetInstance()->IsCombinedSym(curModule, curPkgName, pkgSyms.first)) {
                callback(pkgSyms.first, sym);
            }
        }
    }
}
// LCOV_EXCL_STOP
void MemIndex::FindComment(const Symbol &sym, std::vector<std::string> &comments)
{
    const auto [leadCommentGroup, innerCommentGroup, trailCommentGroup] = sym.comments;
    for (const auto &innerComment : innerCommentGroup) {
        for (const auto &comment : innerComment.cms) {
            comments.push_back(comment.info.Value());
        }
    }
    for (auto &leadComment : leadCommentGroup) {
        for (const auto &comment : leadComment.cms) {
            comments.push_back(comment.info.Value());
        }
    }
    for (auto &trailComment : trailCommentGroup) {
        for (const auto &comment : trailComment.cms) {
            comments.push_back(comment.info.Value());
        }
    }
}

void MemIndex::FindCrossSymbolByName(const std::string &packageName, const std::string &symName, bool isComebined,
    const std::function<void(const CrossSymbol &)> &callback)
{
    std::unordered_set<std::string> targetPackageSet;
    targetPackageSet.insert(packageName);
    if (isComebined) {
        // LCOV_EXCL_START
        auto pkgNameList = CompilerCangjieProject::GetInstance()->GetPkgNameList();
        for (auto &pkgName : pkgNameList) {
            if (pkgName.find(packageName) != std::string::npos) {
                targetPackageSet.insert(pkgName);
            }
        }
        // LCOV_EXCL_STOP
    }
    for (const auto &pkgName : targetPackageSet) {
        if (pkgCrossSymsMap.find(pkgName) == pkgCrossSymsMap.end()) {
            continue;
        }
        for (const auto &crs : pkgCrossSymsMap[pkgName]) {
            if (crs.name != symName) {
                continue;
            }
            callback(crs);
        }
    }
}
void MemIndex::GetExportSID(IDArray array, std::function<void(const CrossSymbol &)> callback) const
{
    (void)array;
    (void)callback;
}

void MemIndex::ForEachFileSymbol(const std::string &pkgName, const std::string &filePath,
    std::function<bool(const Symbol &)> callback) const
{
    auto it = pkgSymsMap.find(pkgName);
    if (it == pkgSymsMap.end()) {
        return;
    }
    for (const auto &sym : it->second) {
        bool isTargetFile = (sym.location.fileUri == filePath);
        if (!isTargetFile) {
            // Check if this is a .macrocall file corresponding to the target file
            if (EndsWith(sym.location.fileUri, ".macrocall")) {
                // Extract source file path from .macrocall path
                // e.g., "foo.cj.macrocall" -> "foo.cj"
                std::string macroCallFile = sym.location.fileUri;
                std::string sourceFile = macroCallFile.substr(0, macroCallFile.length() - 10); // Remove ".macrocall"
                isTargetFile = (sourceFile == filePath);
            }
        }
        if (isTargetFile) {
            if (!callback(sym)) {
                return;
            }
        }
    }
}

bool MemIndex::HasSymbolReference(SymbolID symId) const
{
    for (const auto &pkgRefs : pkgRefsMap) {
        auto it = pkgRefs.second.find(symId);
        if (it == pkgRefs.second.end()) {
            continue;
        }
        for (const auto &ref : it->second) {
            if ((static_cast<uint8_t>(ref.kind) & static_cast<uint8_t>(RefKind::REFERENCE)) != 0) {
                return true;
            }
        }
    }
    return false;
}

bool MemIndex::IsSymbolOverridden(SymbolID symId) const
{
    for (const auto &pkgRelations : pkgRelationsMap) {
        for (const auto &rel : pkgRelations.second) {
            if (rel.predicate == RelationKind::RIDDEND_BY && rel.object == symId) {
                return true;
            }
        }
    }
    return false;
}

SymbolID MemIndex::GetSymbolContainerId(SymbolID symId) const
{
    for (const auto &pkgRelations : pkgRelationsMap) {
        for (const auto &rel : pkgRelations.second) {
            if (rel.predicate == RelationKind::CONTAINED_BY && rel.subject == symId) {
                return rel.object;
            }
        }
    }
    return INVALID_SYMBOL_ID;
}

bool MemIndex::HasReferencedConstructorChild(SymbolID symId) const
{
    std::unordered_set<SymbolID> subjects;
    for (const auto &pkgRelations : pkgRelationsMap) {
        for (const auto &rel : pkgRelations.second) {
            if (rel.predicate == RelationKind::CONTAINED_BY && rel.object == symId) {
                (void)subjects.insert(rel.subject);
            }
        }
    }
    if (subjects.empty()) {
        return false;
    }
    for (const auto &pkgSyms : pkgSymsMap) {
        for (const auto &sym : pkgSyms.second) {
            if (subjects.count(sym.id) > 0 &&
                sym.symInfo.kind == lsp::SymbolKind::CONSTRUCTOR &&
                HasSymbolReference(sym.id)) {
                return true;
            }
        }
    }
    return false;
}

} // namespace lsp
} // namespace ark
