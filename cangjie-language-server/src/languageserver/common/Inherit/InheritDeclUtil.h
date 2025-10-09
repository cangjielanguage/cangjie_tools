// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_INHERITDECLUTIL_H
#define LSPSERVER_INHERITDECLUTIL_H

#include "cangjie/AST/Node.h"
#include "../../../json-rpc/Common.h"
#include "../../../json-rpc/CompletionType.h"

namespace ark {
class InheritDeclUtil {
public:
    explicit InheritDeclUtil(Ptr<const Cangjie::AST::Decl> iDecl) : inDecl(iDecl) {};

    void HandleFuncDecl(bool isDocumentHighlight = false);

    std::set<Ptr<Cangjie::AST::Decl> > GetRelatedFuncDecls() const
    {
        return funcDecls;
    };
private:
    template<class T>
    void HandleDeclBody(T *decl);

    template<class T>
    void HandleDeclBodyForProp(T *decl);

    void HandleRelatedFuncDeclsFromTopLevel(Ptr<Cangjie::AST::Decl> topLevel, bool needSub = true);

    Ptr<const Cangjie::AST::Decl> inDecl{nullptr};
    Ptr<const Cangjie::AST::FuncDecl> defaultFuncDecl{nullptr};
    Ptr<const Cangjie::AST::PropDecl> defaultPropDecl{nullptr};
    std::set<Ptr<Cangjie::AST::Decl> > funcDecls{};
    bool isRename = false;
    std::string pkgName = "";
    std::string newName = "";
    std::string editPkgPath = "";
    std::map<std::string, bool> superDecls = {};
    std::set<Location> References{};
    std::unordered_map<std::string, std::set<TextEdit>> defineEditMap{};
    std::unordered_map<std::string, std::set<TextEdit>> usersEditMap{};

    void GetRefInfoFromFuncDecl();

    void GetReNameInfoFromFuncDecl();

    void addDeclToRef(Ptr<Cangjie::AST::Decl> const &decl, int length);

    void InsertRefUsers(std::unordered_set<Ptr<Cangjie::AST::Node> > &users);

    void InsertRenameUsers(const std::string &definedPath, std::unordered_set<Ptr<Cangjie::AST::Node> > &users);

    void DealTopClass(std::vector<Ptr<Cangjie::AST::InheritableDecl> > &topClasses);
};
} // namespace ark

#endif // LSPSERVER_INHERITDECLUTIL_H
