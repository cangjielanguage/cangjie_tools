// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef DSL_DSL_H
#define DSL_DSL_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <set>

#include "cangjie/AST/ASTCasting.h"
#include "cangjie/AST/Node.h"
#include "cangjie/AST/Types.h"
#include "cangjie/AST/Utils.h"
#include "cangjie/AST/Walker.h"
#include "cangjie/Mangle/BaseMangler.h"
#include "cangjie/Sema/TypeManager.h"

using namespace Cangjie;
using namespace AST;

class Dsl {
public:
    Dsl(TypeManager& typeMan1, TypeManager& typeMan2, bool checkAPI, bool checkABI)
        : typeMan1(typeMan1), typeMan2(typeMan2), checkAPI(checkAPI), checkABI(checkABI)
    {
    }

    /***** Helper Macros *****/

#define CHECK(rule, expr, symbols...)                                                                                  \
    do {                                                                                                               \
        assert(checker.Checks(rule) && "Rule does not belong to checker!");                                            \
        if ((Rules::GetRule(rule).IsAPI() && dsl.checkAPI) || (Rules::GetRule(rule).IsABI() && dsl.checkABI)) {        \
            auto result = expr;                                                                                        \
            checkerResult = checkerResult && result;                                                                   \
            logger.LogIfFalse(rule, {symbols}, result);                                                                \
        }                                                                                                              \
    } while (0)

#define BEGIN_FORALL(var, collection, predicate)                                                                       \
    for (auto var : collection) {                                                                                      \
        if (!(predicate)) {                                                                                            \
            continue;                                                                                                  \
        }
#define END_FORALL() }

#define LETIF(var, function, predicate)                                                                                \
    auto var = function;                                                                                               \
    if (!(predicate)) {                                                                                                \
        continue;                                                                                                      \
    }

    /***** Utils *****/

    /** @brief Returns true if `modifiers` contains `kind`, false otherwise. */
    static bool HasModifier(const std::set<Modifier>& modifiers, TokenKind kind);

    std::string GetMangledName(Decl* x);

    /***** Predicates *****/

    /** @brief Returns true if `x` is a var, let, or const node, false otherwise. */
    static bool VarLetOrConst(const Node* x);

    /** @brief Returns true if `x` is initialized, false otherwise. */
    static bool IsInitialized(const Node* x);

    /** @brief Returns true if `x` is a ImportSpec node, false otherwise. */
    static bool Import(const Node* x);

    /** @brief Returns true if `x` is a func node, false otherwise. */
    static bool Func(const Node* x);

    /** @brief Returns true if `x` is a prop node, false otherwise. */
    static bool PropDecl(const Node* x);

    /** @brief Returns true if `x` is a struct node, false otherwise. */
    static bool StructDecl(const Node* x);

    /** @brief Returns true if `x` is an enum node, false otherwise. */
    static bool EnumDecl(const Node* x);

    /** @brief Returns true if `x` is a class node, false otherwise. */
    static bool ClassDecl(const Node* x);

    /** @brief Returns true if `x` is an interface node, false otherwise. */
    static bool InterfaceDecl(const Node* x);

    /** @brief Returns true if `x` is an extend node, false otherwise. */
    static bool ExtendDecl(const Node* x);

    /** @brief Returns true if `x` is a type alias decl, false otherwise. */
    static bool TypeAlias(const Node* x);

    /** @brief Returns true if `x` is a top-level declaration, false otherwise. */
    static bool TopLevel(const Node* x);

    /** @brief Returns true if `f1` and `f2` are function declarations and if `f1` has the same number of parameters as
     * `f2`, false otherwise. */
    static bool SameNumberParams(const Node* f1, const Node* f2);

    /** @brief Returns true if `f1` and `f2` are marked as const or Frozen, false otherwise. */
    static bool IsConstOrFrozenFunc(const Node* f1, const Node* f2);

    /** @brief Returns true if `t1` and `t2` are the same type. */
    bool SameType(const Ty* ty1, const Ty* ty2) const;

    /** @brief Returns true if `x` has public visibility. */
    static bool IsPublic(const Node* x);
    static bool IsProtected(const Node* x);
    static bool IsPublicOrProtected(const Node* x);
    static bool IsForeign(const Node* x);
    static bool IsStatic(const Node* x);
    static bool IsMut(const Node* x);
    static bool IsOpen(const Node* x);

    /** @brief Returns true if `t1` is a subtype of `t2`. False otherwise. */
    bool Subtype(Ty* t1, Ty* t2) const;

    /** @brief Returns true if t2 is the parent type of t1. */
    bool IsParentType(Ty* t2, Ty* t1) const;

    /** @brief Returns true if a `FuncParam` node `n1` and is a named parameter. */
    static bool IsNamedParam(const Node* n1);

    /** @brief Returns true if a `FuncParam` node `n1` and is a member variable parameter. */
    static bool IsMemberParam(const Node* n1);

    /** @brief Returns true if a `FuncParam` node `n1` has a default value. */
    static bool HasDefaultValue(const Node* n1);

    /** @brief Returns if `x1` has deprecated annotation and if the annotation is a true strict. */
    static std::tuple<bool, bool> CheckDepreAnnotation(const Node* x1);

    /** @brief Returns if `x1` has C annotation and if the annotation is marked as unsafe. */
    static std::tuple<bool, bool> CheckCAnnotation(const Node* x1);

    /** @brief Returns if `x1` annotation CallingConv is marked as CDECL or STDCALL. */
    static std::tuple<bool, bool> CheckCallingConvAnnotation(const Node* x1);

    /** @brief Returns if `x1` annotation Frozen is marked. */
    static bool IsFrozen(const Node* x1);

    /** @brief Returns true if `x` is a var. */
    static bool IsVar(const Node* x);

    /** @brief Returns true if `x1` and `x2` have the same nameless non public Decls. */
    bool SameNamelessDecls(const Node* x1, const Node* x2);

    /** @brief Returns true if `x1` and `x2` have the same nameless non public Decl. */
    bool SameNamelessDecl(const Node* x1, const Node* x2, bool isClass = false);

    /** @brief Returns true if `x1` and `x2` have the same value. */
    bool SameVarValue(const Node* x1, const Node* x2);

    /** @brief Returns true if `x1` and `x2` have the same value. */
    bool SameVarValueCallExpr(const CallExpr* x1, const CallExpr* x2);

    /** @brief Returns true if `node` have the const or Frozen init function. */
    bool HasConstOrFrozenInit(Node* node);

    /** @brief Returns true if `x` is a let. */
    static bool IsLet(const Node* x);

    /** @brief Returns true if `x` is a const. */
    static bool IsConst(const Node* x);

    /** @brief Returns true if `n1` and `n2` are imports with the same package. */
    static bool SameImportPackage(Node* n1, Node* n2);

    /** @brief Returns true if `n1` and `n2` are declarations with the same identifier. */
    static bool SameIdentifier(Node* n1, Node* n2);

    /** @brief Returns true if `n1` and `n2` nodes are the same function. */
    bool SameFunc(Node* n1, Node* n2);

    static bool IsClass(const OwnedPtr<Type>& x);

    static bool IsInterface(const OwnedPtr<Type>& x);

    /** @brief Returns true if and only if `ty` is a Class type. */
    static bool IsClassLikeTy(const Ty* ty);

    /***** Functions *****/

    /** @brief Returns the number of parameters if `n` is a function declaration, 0 otherwise. */
    static size_t NumFuncParams(const Node* n);

    /** @brief Returns the number of MemberParam if `n` is a constructor function declaration, 0 otherwise. */
    static size_t NumMemberParams(const Node* n);

    /** @brief Returns the `i`th parameter of function `n` if `n` is a function and has more than `i` parameters,
     * nullptr otherwise */
    static Node* GetFuncParam(const Node* n, size_t i);

    /** @brief Returns the number of generic parameters if `n` is a declaration with generic, 0 otherwise. */
    static size_t GetNumGenericParams(const Node* n);

    /** @brief Returns the number of generic constraints if `n` is a declaration with generic, 0 otherwise. */
    static size_t GetNumGenericConstraints(const Node* n);

    /** @brief Returns the number of generic upperBounds if `n` is a declaration with generic, 0 otherwise. */
    static size_t GetNumGenericUpperBounds(const Node* n);

    /** @brief Returns the generic of node if `n` is a declaration with generic, nullptr otherwise. */
    static Ptr<Generic> GetGeneric(const Node* n);

    /** @brief Returns the type of node `n`. */
    static Ty* TypeOf(const Node* n);

    /** @brief Returns the return type of `n` if `n` is a function, nullptr otherwise. */
    static Ty* ReturnType(const Node* n);

    /** @brief Returns the target type of a type alias declaration. */
    static Ty* AliasTargetTy(const Node* n);

    static std::string NameOfParam(const Node* n);

    /** @brief Returns the hash of `n` if `n` is a `Decl`, 0 otherwise. */
    static size_t BodyHashOf(const Node* n);

    /** @brief Returns true if v is at the end of declList without considering items in the declMap. */
    static bool IsAtEnd(Node* v, std::set<Node*> declMap, const std::vector<Ptr<Cangjie::AST::Decl>> declList);

    static size_t GetInheritedTypesSize(const Node* x, const std::function<bool(const OwnedPtr<Type>&)> f);

    static std::unordered_set<std::string> GetInheritedTypes(
        const Node* x, const std::function<bool(const OwnedPtr<Type>&)> f);

    static std::unordered_set<std::string> GetInheritedTypesFromExtendDecl(const Node* ed);

    bool HasUnimplementedInterface(const Node* i1, const Node* i2);
    bool HasOverridingFunc(const OwnedPtr<Type>& itype, Node* fd);
    bool HasOverridingDecl(Node* n, Node* f);
    bool HasUnimplementedFunc(const OwnedPtr<Type>& itype, Node* fd);
    bool HasUnimplementedDecl(Node* n1, Node* f1);
    bool IsSameFuncDecl(Node* n1, Node* n2);
    bool CheckUnimplementedFuncDecl(const OwnedPtr<Type>& itype, std::vector<Decl*>& decls);

    /***** Set Utils *****/

    /** @brief Runs `f` on `nodes`. Returns true if `f(x) == true`, for all `x in nodes`, false otherwise. */
    static bool Forall(std::set<Node*> nodes, std::function<bool(Node*)> f);

    /** @brief Runs `f` on `node`. Returns true if `f(node) == true`, false otherwise. */
    static bool Forall(Node* node, std::function<bool(Node*)> f);

    /** @brief Runs `f` on `values`. Returns true if `f(x) == true`, for all `x in values`, false otherwise. */
    static bool Forall(std::set<size_t> values, std::function<bool(size_t)> f);

    /** @brief Returns the oldest node of the pair */
    static const Node* GetOldNode(const Node* oldNode, const Node* newNode);

    /** @brief Returns the newest node of the pair */
    static const Node* GetNewNode(const Node* oldNode, const Node* newNode);

    /** @brief Returns a set containing size_t elements in range [begin, end[. */
    static std::set<size_t> Range(size_t begin, size_t end);

    /** @brief Returns `map[node]` if it exists, nullptr otherwise. */
    static Node* Corresponding(Node* node, const std::map<Node*, Node*>& map);

    /***** Options *****/

    /** @brief Check for API rules. */
    const bool checkAPI;
    /** @brief Check for ABI rules. */
    const bool checkABI;

private:
    TypeManager& typeMan1;
    TypeManager& typeMan2;
}; // class Dsl

#endif // DSL_DSL_H
