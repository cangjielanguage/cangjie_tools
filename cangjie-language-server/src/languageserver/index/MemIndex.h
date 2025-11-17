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
};

struct LookupRequest {
    std::unordered_set<SymbolID> ids;
};

struct RefsRequest {
    std::unordered_set<SymbolID> ids;
    RefKind filter = RefKind::ALL;
};

struct RelationsRequest {
    SymbolID id;
    RelationKind predicate;
};

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

    void FindImportSymsOnCompletion(const std::unordered_set<ark::lsp::SymbolID> &normalCompleteSyms,
        const std::unordered_set<ark::lsp::SymbolID> &importDeclSyms, const std::string &curPkgName,
        const std::string &curModule, const std::function<void(const std::string &, const Symbol &)>& callback);

    void FindExtendSymsOnCompletion(const ark::lsp::SymbolID &dotCompleteSym,
        const std::unordered_set<lsp::SymbolID> &visibleMembers,
        const std::string &curPkgName, const std::string &curModule,
        const std::function<void(const std::string &, const std::string &, const Symbol &)>& callback);

    void RefsFindReference(const RefsRequest &req, Ref &definition, std::function<void(const Ref &)> callback) const;

    void FindImportSymsOnQuickFix(const std::string &curPkgName, const std::string &curModule,
        const std::unordered_set<ark::lsp::SymbolID> &importDeclSyms,
        const std::string& identifier, const std::function<void(const std::string &, const Symbol &)>& callback);

    std::map<std::string, SymbolSlab> pkgSymsMap{};

    std::map<std::string, RefSlab> pkgRefsMap{};

    std::map<std::string, RelationSlab> pkgRelationsMap{};

    std::map<std::string, ExtendSlab> pkgExtendsMap{};

    void FindRiddenUp(SymbolID id, std::unordered_set<SymbolID> &ids, SymbolID &topId)
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

    void FindRiddenDown(SymbolID id, std::unordered_set<SymbolID> &ids)
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

    Symbol GetAimSymbol(const Decl &decl);
};

} // namespace lsp
} // namespace ark

#endif // LSPSERVER_MEMINDEX_INDEX_H