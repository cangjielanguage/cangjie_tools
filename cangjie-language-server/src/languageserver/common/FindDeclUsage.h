// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_FIND_DECL_USAGE
#define CANGJIE_FIND_DECL_USAGE

#include <unordered_set>
#include "cangjie/AST/Node.h"
#include "cangjie/AST/Walker.h"
#include "Constants.h"

namespace ark {
using namespace Cangjie;
using namespace AST;
bool CheckFunctionEqual(const FuncDecl& srcFunc, const FuncDecl& targetFunc);
bool CheckTypeEqual(Ty& src, Ty& target);
bool CheckDeclEqual(const Decl& source, const Decl& target);
std::unordered_set<Ptr<Node> > FindDeclUsage(const Decl &decl, Node &root, bool isRename = false);
}
#endif