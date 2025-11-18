// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "InheritImpl.h"
#include "../Utils.h"

using namespace Cangjie;
// LCOV_EXCL_START
namespace ark {
void HandleSuperDecl(std::queue<Ptr<InheritableDecl> > &queues, InheritableDecl &inheritableDecl,
                     std::vector<Ptr<InheritableDecl> > &topClasses, std::vector<Ptr<InheritableDecl> > &libClasses)
{
    size_t invalidDeclCount = 0;
    for (auto &it : inheritableDecl.inheritedTypes) {
        if (it->ty == nullptr) {
            invalidDeclCount++;
            continue;
        }
        Ptr<ClassLikeDecl> superDecl = nullptr;
        if (it->ty->kind == TypeKind::TYPE_CLASS) {
            superDecl = dynamic_cast<ClassTy *>(it->ty.get())->decl;
        } else if (it->ty->kind == TypeKind::TYPE_INTERFACE) {
            superDecl = dynamic_cast<InterfaceTy *>(it->ty.get())->decl;
        }
        if (!superDecl) {
            invalidDeclCount++;
            continue;
        }
        if (IsFromSrcOrNoSrc(superDecl)) {
            queues.push(superDecl);
        } else {
            libClasses.push_back(superDecl);
            invalidDeclCount++;
        }
    }

    if (invalidDeclCount == inheritableDecl.inheritedTypes.size()) {
        topClasses.push_back(&inheritableDecl);
    }
}

std::vector<Ptr<InheritableDecl> > GetTopClassDecl(InheritableDecl &classLikeOrStructDecl, bool isLib)
{
    std::vector<Ptr<InheritableDecl> > topClasses{};
    std::vector<Ptr<InheritableDecl> > libClasses{};
    std::queue<Ptr<InheritableDecl> > queues{};
    queues.push(&classLikeOrStructDecl);
    while (!queues.empty()) {
        auto topClass = queues.front();
        queues.pop();
        if (topClass) {
            HandleSuperDecl(queues, *topClass, topClasses, libClasses);
        }
    }
    if (isLib) {
        return libClasses;
    }
    return topClasses;
}
} // namespace ark
// LCOV_EXCL_STOP
