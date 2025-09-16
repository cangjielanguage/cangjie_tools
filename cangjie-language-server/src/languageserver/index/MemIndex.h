// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_MEMINDEX_INDEX_H
#define LSPSERVER_MEMINDEX_INDEX_H

#include <cstdint>
#include <functional>
#include <unordered_set>
#include "../common/Utils.h"
#include "Ref.h"
#include "Relation.h"
#include "Symbol.h"

namespace ark {
namespace lsp {

struct FuzzyFindRequest {
    std::string query;
    /// If this is non-empty, symbols must be in at least one of the scopes
    /// (e.g. namespaces) excluding nested scopes. For example, if a scope "xyz::"
    /// is provided, the matched symbols must be defined in namespace xyz but not
    /// namespace xyz::abc.
    ///
    /// The global scope is "", a top level scope is "foo::", etc.
    std::vector<std::string> scopes;
    /// If set to true, allow symbols from any scope. Scopes explicitly listed
    /// above will be ranked higher.
    bool anyScope = false;
    /// The number of top candidates to return. The index may choose to
    /// return more than this, e.g. if it doesn't know which candidates are best.
    std::optional<uint32_t> limit;
    /// If set to true, only symbols for completion support will be considered.
    bool restrictForCodeCompletion = false;
    /// Contextually relevant files (e.g. the file we're code-completing in).
    /// Paths should be absolute.
    std::vector<std::string> proximityPaths;
    /// Preferred types of symbols. These are raw representation of `OpaqueType`.
    std::vector<std::string> preferredTypes;

    bool operator==(const FuzzyFindRequest &req) const
    {
        return std::tie(query, scopes, limit, restrictForCodeCompletion,
                        proximityPaths, preferredTypes) ==
               std::tie(req.query, req.scopes, req.limit,
                        req.restrictForCodeCompletion, req.proximityPaths,
                        req.preferredTypes);
    }
    bool operator!=(const FuzzyFindRequest &req) const { return !(*this == req); }
};

struct LookupRequest {
    std::unordered_set<SymbolID> ids;
};

struct RefsRequest {
    std::unordered_set<SymbolID> ids;
    RefKind filter = RefKind::ALL;
    std::optional<uint32_t> limit;
};

struct RelationsRequest {
    SymbolID id;
    RelationKind predicate;
    /// If set, limit the number of relations returned from the index.
    std::optional<uint32_t> limit;
};

struct FuncSigRequest {
    SymbolLanguage lang;
    std::string funcName;
    std::string retType;
    std::vector<std::string> funcParams;
};

enum class PackageRelation { NONE, CHILD, PARENT, SAME_MODULE };
PackageRelation GetPackageRelation(const std::string &srcFullPackageName,
                                   const std::string &targetFullPackageName);

/// Interface for symbol indexes that can be used for searching
class SymbolIndex {
public:
    virtual ~SymbolIndex() = default;

    virtual void FuzzyFind(const FuzzyFindRequest &req, std::function<void(const Symbol &)> callback) const = 0;

    virtual void Lookup(const LookupRequest &req, std::function<void(const Symbol &)> callback) const = 0;

    virtual void Refs(const RefsRequest &req, std::function<void(const Ref &)> callback) const = 0;

    virtual void Callees(const std::string &pkgName, const SymbolID &declId,
        std::function<void(const SymbolID &, const Ref &)> callback) const = 0;

    virtual void Relations(const RelationsRequest &req, std::function<void(const Relation &)> callback) const = 0;

    virtual void GetFuncByName(const FuncSigRequest &req,
                                std::function<void(const Symbol &)> callback) const = 0;

    virtual Symbol GetAimSymbol(const Decl& decl) = 0;

    virtual void FindRiddenUp(SymbolID id, std::unordered_set<SymbolID> &ids, SymbolID &topId) = 0;

    virtual void FindRiddenDown(SymbolID id, std::unordered_set<SymbolID> &ids) = 0;

    virtual void FindImportSymsOnCompletion(
        const std::pair<std::unordered_set<SymbolID>, std::unordered_set<SymbolID>>& filterSyms,
        const std::string &curPkgName, const std::string &curModule, const std::string &prefix,
        std::function<void(const std::string &, const Symbol &, const CompletionItem &)> callback) = 0;

    virtual void FindImportSymsOnQuickFix(const std::string &curPkgName, const std::string &curModule,
        const std::unordered_set<SymbolID> &importDeclSyms,
        const std::string& identifier,
        const std::function<void(const std::string &, const Symbol &)>& callback) = 0;

    virtual void FindExtendSymsOnCompletion(const SymbolID &dotCompleteSym,
        const std::unordered_set<SymbolID> &visibleMembers,
        const std::string &curPkgName, const std::string &curModule,
        const std::function<void(const std::string &, const std::string &,
            const Symbol &, const CompletionItem &)>& callback) =0;

    virtual void FindComment(const Symbol &sym, std::vector<std::string> &comments) = 0;

    virtual void RefsFindReference(const RefsRequest &req, Ref &definition,
        std::function<void(const Ref &)> callback) const = 0;

    virtual void FindCrossSymbolByName(const std::string &packageName, const std::string &symName, bool isComebined,
        const std::function<void(const CrossSymbol &)> &callback) = 0;
};

class MemIndex : public SymbolIndex {
public:
    MemIndex() = default;

    void FuzzyFind(const FuzzyFindRequest &req, std::function<void(const Symbol &)> callback) const override;

    void Lookup(const LookupRequest &req, std::function<void(const Symbol &)> callback) const override;

    void Refs(const RefsRequest &req, std::function<void(const Ref &)> callback) const override;

    void Callees(const std::string &pkgName, const SymbolID &declId,
        std::function<void(const SymbolID &, const Ref &)> callback) const override;

    void Relations(const RelationsRequest &req, std::function<void(const Relation &)> callback) const override;

    void FindImportSymsOnCompletion(
        const std::pair<std::unordered_set<SymbolID>, std::unordered_set<SymbolID>>& filterSyms,
        const std::string &curPkgName, const std::string &curModule, const std::string &prefix,
        std::function<void(const std::string &, const Symbol &, const CompletionItem &)> callback) override;

    void FindExtendSymsOnCompletion(const SymbolID &dotCompleteSym,
        const std::unordered_set<SymbolID> &visibleMembers,
        const std::string &curPkgName, const std::string &curModule,
        const std::function<void(const std::string &, const std::string &,
            const Symbol &, const CompletionItem &)>& callback) override;

    void RefsFindReference(const RefsRequest &req, Ref &definition,
        std::function<void(const Ref &)> callback) const override;

    void FindImportSymsOnQuickFix(const std::string &curPkgName, const std::string &curModule,
        const std::unordered_set<SymbolID> &importDeclSyms,
        const std::string& identifier,
        const std::function<void(const std::string &, const Symbol &)>& callback) override;

    void FindComment(const Symbol &sym, std::vector<std::string> &comments) override;

    void FindCrossSymbolByName(const std::string &packageName, const std::string &symName, bool isComebined,
        const std::function<void(const CrossSymbol &)> &callback) override;

    std::map<std::string, SymbolSlab> pkgSymsMap{};

    std::map<std::string, RefSlab> pkgRefsMap{};

    std::map<std::string, RelationSlab> pkgRelationsMap{};

    std::map<std::string, ExtendSlab> pkgExtendsMap{};

    std::map<std::string, CrossSymbolSlab> pkgCrossSymsMap{};

    void FindRiddenUp(SymbolID id, std::unordered_set<SymbolID> &ids, SymbolID &topId) override
    {
        for (const auto &relations : pkgRelationsMap) {
            for (const auto &rel : relations.second) {
                if (rel.predicate == RelationKind::RIDDEND_BY && rel.object == id) {
                    (void)ids.emplace(rel.subject);
                    topId = rel.subject;
                    FindRiddenUp(rel.subject, ids, topId);
                }
            }
        }
    }

    void FindRiddenDown(SymbolID id, std::unordered_set<SymbolID> &ids) override
    {
        for (const auto &relations : pkgRelationsMap) {
            for (const auto &rel : relations.second) {
                if (rel.predicate == RelationKind::RIDDEND_BY && rel.subject == id) {
                    (void)ids.emplace(rel.object);
                    FindRiddenDown(rel.object, ids);
                }
            }
        }
    }

    Symbol GetAimSymbol(const Decl &decl) override;
    void GetFuncByName(const FuncSigRequest &req,
                       std::function<void(const Symbol &)> callback) const override {}
};

} // namespace lsp
} // namespace ark

#endif // LSPSERVER_MEMINDEX_INDEX_H