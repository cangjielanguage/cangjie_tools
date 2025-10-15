// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef SELECTIONTREE_H
#define SELECTIONTREE_H
#include <cangjie/AST/ASTContext.h>
#include "../ArkAST.h"

namespace ark {
using namespace Cangjie;
using namespace AST;
class SelectionTree {
public:
    static bool CreateEach(const ArkAST &arkAST,
        const std::string &fileName,
        Position Begin,
        Position End,
        std::function<bool(SelectionTree)> Func);

    enum class Selection : unsigned char {
        Unselected,
        Partial,
        Complete,
    };

    // can only be refact within scope: GLOBAL_VAR/MEMBER_VAR/FUNC_BODY
    enum class Scope {
        UNKNOWN,
        GLOBAL_VAR,
        MEMBER_VAR,
        FUNC_BODY
    };

    struct SelectionTreeNode {
        Ptr<Node> node;
        Ptr<Node> Parent;
        std::vector<Ptr<SelectionTreeNode>> Children;
        Selection selected;
    };

    enum class WalkAction {
        WALK_CHILDREN,
        SKIP_CHILDREN,
        STOP_NOW
    };

    const SelectionTreeNode *CommonAncestor() const;

    const SelectionTreeNode *root() const
    {
        return Root ? &(*Root) : nullptr;
    }

    const Scope SelectedScope() const
    {
        return scope;
    }

    const Ptr<Cangjie::AST::Node> TargetDecl() const
    {
        return targetDecl;
    }

    const Ptr<Cangjie::AST::Node> TopDecl() const
    {
        return topDecl;
    }

    const Ptr<Cangjie::AST::Node> OuterInterpExpr() const
    {
        return outerInterpExpr;
    }

    using WalkCallBack = std::function<WalkAction(const SelectionTreeNode*)>;

    static void Walk(const SelectionTreeNode *node, WalkCallBack callBack) ;

    void printSelection(const SelectionTreeNode* node, int level = 0) const;

private:
    // Creates a selection tree for the given range in the main file.
    // The range includes bytes [start, end).
    SelectionTree(const ArkAST &arkAST, const std::string &fileName, Position start, Position end);

    bool FindSelectNode(Decl *decl, Position start, Position end);

    void MatchSelectedScope(Ptr<Node> node, Position start, Position end);

    void FindTopDecl(Cangjie::AST::Node &node);

    static void BuildTreeNode(SelectionTreeNode *rootTreeNode, Position start, Position end);

    static WalkAction WalkImpl(const SelectionTreeNode* node, WalkCallBack callBack);

    std::unique_ptr<SelectionTreeNode> Root;

    Cangjie::AST::Decl *topDecl = nullptr;

    Scope scope = Scope::UNKNOWN;

    Ptr<Cangjie::AST::Node> targetDecl = nullptr;

    Ptr<Cangjie::AST::Node> outerInterpExpr = nullptr;
};
} // namespace ark

#endif // SELECTIONTREE_H
