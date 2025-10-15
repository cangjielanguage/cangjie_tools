// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_LSP_TWEAKUTILS_H
#define CANGJIE_LSP_TWEAKUTILS_H

#include "cangjie/AST/Node.h"
#include "cangjie/AST/ASTContext.h"
#include "../../ArkAST.h"
#include "../../common/Utils.h"
#include "../../selection/SelectionTree.h"

namespace ark {
using namespace Cangjie;
using namespace AST;

class TweakUtils {
public:
    static Ptr<Block> FindOuterScopeNode(const ASTContext& ctx, const Expr& expr, bool &isGlobal, Range &range);

    static bool Contain(Cangjie::AST::Node &node, Range &range);

    static bool CheckValidExpr(const SelectionTree::SelectionTreeNode &treeNode);

    static Position FindLastImportPos(const File &file);

    static Range FindGlobalInsertPos(const File &file, Range &range);
private:
    static Ptr<Block> GetSatisfiedBlock(const ASTContext &ctx, Range &range, const std::string &scopeName,
        bool &isGlobal);

    static Ptr<Block> GetSatisfiedBlockBySearch(const ASTContext &ctx, Range &range, const std::string &scopeName,
        bool &isGlobal);

    static Ptr<Block> GetSymbolBlock(AST::Symbol &symbol, Range &range);

    static Ptr<Block> DealTryExpr(TryExpr *tryExpr, Range &range);

    static Ptr<Block> DealBlock(Block *block, Range &range);

    static Ptr<Block> DealFuncBody(FuncBody *funcBody, Range &range);

    static Ptr<Block> DealIfExpr(IfExpr *ifExpr, Range &range);

    static Ptr<Block> DealWhileExpr(WhileExpr *whileExpr, Range &range);

    static Ptr<Block> DealDoWhileExpr(DoWhileExpr *doWhileExpr, Range &range);

    static Ptr<Block> DealForInExpr(ForInExpr *forInExpr, Range &range);

    static Ptr<Block> DealMatchCase(MatchCase *matchCase, Range &range);

    static Ptr<Block> DealFuncDecl(FuncDecl *funcDecl, Range &range);
};
} // namespace ark

#endif // CANGJIE_LSP_TWEAKUTILS_H
