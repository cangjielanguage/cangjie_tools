// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DIFF_DIFF_H
#define DIFF_DIFF_H

#include <algorithm>
#include <iostream>
#include <string>

#include "cangjie/AST/Node.h"
#include "cangjie/AST/PrintNode.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/Basic/DiagnosticEngine.h"
#include "cangjie/Basic/Match.h"
#include "cangjie/Basic/Print.h"
#include "cangjie/Modules/CjoManager.h"
#include "cangjie/Sema/TypeManager.h"
#include "cangjie/Utils/CastingTemplate.h"
#include "cangjie/Mangle/BaseMangler.h"

using namespace Cangjie;
using namespace Cangjie::AST;

class Diff {
public:
    Diff(const Package& pkg1, const Package& pkg2, TypeManager& typeMan1, TypeManager& typeMan2)
        : pkg1(pkg1), pkg2(pkg2), typeMan1(typeMan1), typeMan2(typeMan2), oldWalkerID(pkg1.visitedByWalkerID)
    {
    }

    void CalculateDiff();

    void Print() const;

    Node* GetPotentiallyModifiedElement(Node* key);
    void SetPotentiallyModifiedElement(Node* key, Node* value);
    bool HasPotentiallyModifiedElement(Decl* key);
    std::set<Node*> GetDeleted() const;
    std::set<Node*> GetAdded() const;
    std::set<Node*> GetDomPotentiallyModified() const;
    std::string ToString();
    const std::map<Node*, Node*>& PotentiallyModified() const;

    std::set<Node*> GetMemberDeleted(Node* key) const;
    std::set<Node*> GetMemberAdded(Node* key) const;
    const std::map<Node*, Node*> GetPotentiallyMemberModified(Node* key) const;
    std::set<Node*> GetDomPotentiallyMemberModified(Node* key) const;

    const Package& GetOldPackage() const
    {
        return pkg1;
    }
    const Package& GetNewPackage() const
    {
        return pkg2;
    }

    bool IsOldNode(const Node* node) const
    {
        if (!node) {
            return false;
        }
        return node->visitedByWalkerID == oldWalkerID ? true : false;
    }

    bool ModuleVisible(const Node* node) const;

    /** @brief Returns true if `n1` and `n2` nodes are the same extend. */
    bool SameExtend(Node* n1, Node* n2) const;

private:
    const Package& pkg1;
    const Package& pkg2;
    const TypeManager& typeMan1;
    const TypeManager& typeMan2;
    const unsigned oldWalkerID;

    /** @brief Added declarations. Forall `d in added`, `d` only exists in the new version of the AST. */
    std::set<Node*> added; // Elements must always be a `AST::Decl*`
    /** @brief Deleted declarations. Forall `d in deleted`, `d` only exists in the old version of the AST. */
    std::set<Node*> deleted; // Elements must always be a `AST::Decl*`
    /** @brief Potentially Modified declarations. Forall `(d1, d2) in modified`, `d1` only exists in the old version of
     * the AST, and `d2` only exists in the new version of the AST. */
    std::map<Node*, Node*> potentiallyModified; // Elements must always be a `AST::Decl*`

    std::set<Node*> domPotentiallyModified;

    struct MemberDiff {
        /** @brief Added member declarations. */
        std::set<Node*> added;
        /** @brief Deleted member declarations. */
        std::set<Node*> deleted;
        /** @brief Potentially modified member declarations. */
        std::map<Node*, Node*> potentiallyModified;
        std::set<Node*> domPotentiallyModified;
    };
    std::map<Node*, MemberDiff> potentiallyMemberModified;
    std::unordered_set<const AST::Node*> visibleNodesOld;
    std::unordered_set<const AST::Node*> visibleNodesNew;

    /** @brief helper function to collect imports */
    void CollectImports();

    void AddDecl(Node* element);
    void AddDelDecl(Node* element);
    bool CompareDecl(Ptr<Decl> declA, Ptr<Decl> declB);
    void CollectDecl(const OwnedPtr<Decl>& decl, std::unordered_map<std::size_t, Ptr<AST::Decl>>& decls);
    void CollectFileDecls(Ptr<const File> file, std::unordered_map<std::size_t, Ptr<Decl>>& decls, bool isOld);
    void CollectMemberDecls(Ptr<AST::Decl> decl, std::unordered_map<std::size_t, Ptr<AST::Decl>>& members);
    void CollectModifiedMemberDecls(Ptr<AST::Decl> declA, Ptr<AST::Decl> declB);
    void CollectModifiedMemberOfExtendDecls();
    void CheckAndInsertVisibleNode(Decl& decl);
    void CheckConstOrFrozenFuncNode(Decl& decl);
    bool ExtendExported(std::vector<OwnedPtr<Type>>& inheritedTypes);
    bool ExtendTypePublic(Decl& decl);
    void CheckMemberVisibleNode(const Decl& decl);
    void InsertVisibleNode(const Decl& decl);
    void InsertVisibleClassLikeSuperDecl(const Decl& decl);
    bool PropAttrCheck(const Decl* decl) const;
    void HandleRefExprVisibility(const RefExpr* refExpr);
    void HandleCallExprVisibility(const CallExpr* callExpr);
    void HandleVarDeclVisibility(const Decl& decl);
    std::string PrettyPrintNode(Ptr<const Node> node);
};

#endif // DIFF_DIFF_H