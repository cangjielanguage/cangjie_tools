// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/Dsl/Dsl.h"
#include <numeric>

using namespace Cangjie;
using namespace Cangjie::AST;

namespace {
static Ptr<Ty> GetTyFromASTType(TypeManager& typeManager, const Ty* ty, const std::vector<Ptr<Ty>>& typeArgs)
{
    auto pDecl = Ty::GetDeclOfTy(ty);
    if (!pDecl) {
        return nullptr;
    }
    switch (pDecl->astKind) {
        case ASTKind::CLASS_DECL: {
            auto cd = static_cast<ClassDecl*>(pDecl.get());
            return typeManager.GetClassTy(*cd, typeArgs);
        }
        case ASTKind::INTERFACE_DECL: {
            auto id = static_cast<InterfaceDecl*>(pDecl.get());
            return typeManager.GetInterfaceTy(*id, typeArgs);
        }
        case ASTKind::STRUCT_DECL: {
            auto sd = static_cast<StructDecl*>(pDecl.get());
            return typeManager.GetStructTy(*sd, typeArgs);
        }
        case ASTKind::ENUM_DECL: {
            auto ed = static_cast<EnumDecl*>(pDecl.get());
            return typeManager.GetEnumTy(*ed, typeArgs);
        }
        default:
            break;
    }
    return pDecl->GetTy();
}

static std::string GetTypeName(TypeManager& typeManager, const Ty* ty, const std::vector<Ptr<Ty>>& typeArgs = {})
{
    if (!ty) {
        return "";
    }
    auto tystr = Ty::ToString(ty);
    if (tystr.find("TypeAlias-") == std::string::npos &&
        tystr.find("Generics-") == std::string::npos) {
        return tystr;
    }
    if (ty->kind == TypeKind::TYPE) {
        // If the ty is a TypeAliasTy, get the actual type name of the alias type.
        auto aliasTy = static_cast<const TypeAliasTy*>(ty);
        if (!aliasTy->declPtr || !aliasTy->declPtr->type || !aliasTy->declPtr->type->GetTy()) {
            return tystr;
        }
        return GetTypeName(typeManager, aliasTy->declPtr->type->GetTy(), ty->typeArgs);
    }
    if (ty->kind == TypeKind::TYPE_FUNC) {
        auto funcTy = static_cast<const FuncTy*>(ty);
        std::string str{"("};
        for (auto& paramTy : funcTy->paramTys) {
            if (&paramTy == &funcTy->paramTys.back()) {
                str += GetTypeName(typeManager, paramTy, typeArgs);
            } else {
                str += GetTypeName(typeManager, paramTy, typeArgs) + ", ";
            }
        }
        str = str + ") -> " + GetTypeName(typeManager, funcTy->retTy, typeArgs);
        return funcTy->IsCFunc() ? "CFunc<" + str + ">" : str;
    }
    if (ty->HasGeneric() && ty->typeArgs.size() == typeArgs.size()) {
        auto instTy = GetTyFromASTType(typeManager, ty, typeArgs);
        if (!instTy || instTy->HasGeneric()) {
            return tystr;
        }
        return GetTypeName(typeManager, instTy);
    }
    return tystr;
}

static bool IsSamePropDecl(Node* n1, Node* n2)
{
    if (!Dsl::PropDecl(n1) || !Dsl::PropDecl(n2)) {
        return false;
    }
    return Dsl::SameIdentifier(n1, n2);
}
}

bool Dsl::HasModifier(const std::set<Modifier>& modifiers, Cangjie::TokenKind kind)
{
    return std::any_of(modifiers.begin(), modifiers.end(), [kind](const auto& it) { return it.modifier == kind; });
}

bool Dsl::VarLetOrConst(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::VAR_DECL;
}

bool Dsl::IsInitialized(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->TestAttr(Attribute::INITIALIZED);
}

bool Dsl::Import(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::IMPORT_SPEC;
}

bool Dsl::Func(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::FUNC_DECL;
}

bool Dsl::PropDecl(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::PROP_DECL;
}

bool Dsl::StructDecl(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::STRUCT_DECL;
}

bool Dsl::EnumDecl(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::ENUM_DECL;
}

bool Dsl::ClassDecl(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::CLASS_DECL;
}

bool Dsl::InterfaceDecl(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::INTERFACE_DECL;
}

bool Dsl::ExtendDecl(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::EXTEND_DECL;
}

bool Dsl::TypeAlias(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->astKind == ASTKind::TYPE_ALIAS_DECL;
}

bool Dsl::TopLevel(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->TestAttr(Attribute::GLOBAL);
}

bool Dsl::SameNumberParams(const Node* f1, const Node* f2)
{
    auto decl1 = dynamic_cast<const FuncDecl*>(f1);
    auto decl2 = dynamic_cast<const FuncDecl*>(f2);
    if (decl1 == nullptr || decl2 == nullptr ||
        decl1->funcBody->paramLists.empty() ||
        decl2->funcBody->paramLists.empty()) {
        return false;
    }

    return decl1->funcBody->paramLists[0]->params.size() == decl2->funcBody->paramLists[0]->params.size();
}

bool Dsl::IsConstOrFrozenFunc(const Node* f1, const Node* f2)
{
    if ((IsConst(f1) && IsConst(f2)) || (IsFrozen(f1) && IsFrozen(f2))) {
        return true;
    }
    return false;
}

bool Dsl::SameType(const Ty* ty1, const Ty* ty2) const
{
    if (ty1 && ty1->HasGeneric() && ty2 && ty2->HasGeneric()) {
        return true;
    }
    return GetTypeName(typeMan1, ty1) == GetTypeName(typeMan2, ty2);
}

bool Dsl::IsPublic(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->TestAttr(Attribute::PUBLIC);
}

bool Dsl::IsProtected(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->TestAttr(Attribute::PROTECTED);
}

bool Dsl::IsPublicOrProtected(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->TestAttr(Attribute::PUBLIC) || x->TestAttr(Attribute::PROTECTED);
}

bool Dsl::IsForeign(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->TestAttr(Attribute::FOREIGN);
}

bool Dsl::IsStatic(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->TestAttr(Attribute::STATIC);
}

bool Dsl::IsMut(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->TestAttr(Attribute::MUT);
}

bool Dsl::IsOpen(const Node* x)
{
    if (!x) {
        return false;
    }
    return x->TestAttr(Attribute::OPEN);
}

bool Dsl::Forall(std::set<Node*> nodes, std::function<bool(Node*)> f)
{
    bool res = true;
    for (auto x : nodes) {
        auto r = f(x);
        res = res && r;
    }
    return res;
}

bool Dsl::Forall(Cangjie::AST::Node* node, std::function<bool(Cangjie::AST::Node*)> f)
{
    if (node == nullptr) {
        return true;
    }
    return f(node);
}

bool Dsl::Forall(std::set<size_t> values, std::function<bool(size_t)> f)
{
    bool res = true;
    for (auto x : values) {
        auto r = f(x);
        res = res && r;
    }
    return res;
}

const Node* Dsl::GetOldNode(const Node* oldNode, const Node* newNode)
{
    if (oldNode == nullptr) {
        return newNode;
    }
    if (newNode == nullptr) {
        return oldNode;
    }
    return (oldNode->visitedByWalkerID < newNode->visitedByWalkerID) ? oldNode : newNode;
}

const Node* Dsl::GetNewNode(const Node* oldNode, const Node* newNode)
{
    if (oldNode == nullptr) {
        return newNode;
    }
    if (newNode == nullptr) {
        return oldNode;
    }
    return (oldNode->visitedByWalkerID > newNode->visitedByWalkerID) ? oldNode : newNode;
}

size_t Dsl::NumFuncParams(const Node* n)
{
    size_t res{0};
    auto funcDecl = dynamic_cast<const FuncDecl*>(n);
    if (funcDecl == nullptr) {
        return res;
    }
    for (auto& paramLists : funcDecl->funcBody->paramLists) {
        res += paramLists->params.size();
    }
    return res;
}

size_t Dsl::NumMemberParams(const Node* n)
{
    size_t res{0};
    auto funcDecl = dynamic_cast<const FuncDecl*>(n);
    if (funcDecl == nullptr) {
        return res;
    }
    for (auto& paramLists : funcDecl->funcBody->paramLists) {
        for (auto& param : paramLists->params) {
            if (IsMemberParam(param.get())) {
                res += 1;
            }
        }
    }
    return res;
}

Node* Dsl::GetFuncParam(const Node* n, size_t i)
{
    auto funcDecl = dynamic_cast<const FuncDecl*>(n);
    if (funcDecl == nullptr ||
        funcDecl->funcBody->paramLists.empty() ||
        i >= funcDecl->funcBody->paramLists[0]->params.size()) {
        return nullptr;
    }

    return funcDecl->funcBody->paramLists[0]->params[i].get();
}

size_t Dsl::GetNumGenericParams(const Node* n)
{
    auto fd = dynamic_cast<const FuncDecl*>(n);
    if (fd && fd->funcBody->generic) {
        return fd->funcBody->generic->typeParameters.size();
    }
    auto decl = dynamic_cast<const Decl*>(n);
    if (decl && decl->generic) {
        return decl->generic->typeParameters.size();
    }
    return 0;
}

size_t Dsl::GetNumGenericConstraints(const Node* n)
{
    auto fd = dynamic_cast<const FuncDecl*>(n);
    if (fd && fd->funcBody->generic) {
        return fd->funcBody->generic->genericConstraints.size();
    }
    auto decl = dynamic_cast<const Decl*>(n);
    if (decl && decl->generic) {
        return decl->generic->genericConstraints.size();
    }
    return 0;
}

size_t Dsl::GetNumGenericUpperBounds(const Node* n)
{
    auto fd = dynamic_cast<const FuncDecl*>(n);
    if (fd && fd->funcBody->generic) {
        size_t size = 0;
        for (auto& gc : fd->funcBody->generic->genericConstraints) {
            size += gc->upperBounds.size();
        }
        return size;
    }
    auto decl = dynamic_cast<const Decl*>(n);
    if (decl && decl->generic) {
        size_t size = 0;
        for (auto& gc : decl->generic->genericConstraints) {
            size += gc->upperBounds.size();
        }
        return size;
    }
    return 0;
}

Ptr<Generic> Dsl::GetGeneric(const Node* n)
{
    auto fd = dynamic_cast<const FuncDecl*>(n);
    if (fd && fd->funcBody) {
        return fd->funcBody->generic.get();
    }
    auto decl = dynamic_cast<const Decl*>(n);
    if (decl) {
        return decl->generic.get();
    }
    return nullptr;
}

bool Dsl::SameImportPackage(Node* n1, Node* n2)
{
    auto import1 = dynamic_cast<const ImportSpec*>(n1);
    auto import2 = dynamic_cast<const ImportSpec*>(n2);
    if (!import1 || !import2 || !import1->content.isDecl || !import2->content.isDecl) {
        return false;
    }
    auto size = import1->content.prefixPaths.size();
    if (size != import2->content.prefixPaths.size()) {
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        if (import1->content.prefixPaths[i] != import2->content.prefixPaths[i]) {
            return false;
        }
    }
    return true;
}

bool Dsl::SameIdentifier(Node* n1, Node* n2)
{
    auto decl1 = dynamic_cast<const Decl*>(n1);
    auto decl2 = dynamic_cast<const Decl*>(n2);
    if (decl1 && decl2 && decl1->identifier.GetRawText() == decl2->identifier.GetRawText()) {
        return true;
    }
    return false;
}

bool Dsl::SameFunc(Node* n1, Node* n2)
{
    auto fd1 = dynamic_cast<const FuncDecl*>(n1);
    auto fd2 = dynamic_cast<const FuncDecl*>(n2);
    if (!fd1 || !fd2) {
        return false;
    }
    if (fd1->identifier.GetRawText() != fd2->identifier.GetRawText()) {
        return false;
    }
    if (fd1->funcBody->paramLists[0]->params.size() != fd2->funcBody->paramLists[0]->params.size()) {
        return false;
    }
    auto paramNum = fd1->funcBody->paramLists[0]->params.size();
    for (size_t i = 0; i < paramNum; i++) {
        if (!SameType(fd1->funcBody->paramLists[0]->params[i]->GetTy(),
            fd2->funcBody->paramLists[0]->params[i]->GetTy())) {
            return false;
        }
    }
    return true;
}

bool Dsl::IsParentType(Ty* t2, Ty* t1) const
{
    if (!t1 || !t2) {
        return false;
    }
    auto superTys = typeMan1.GetAllSuperTys(*t1, {});
    if (superTys.empty()) {
        superTys = typeMan2.GetAllSuperTys(*t1, {});
    }
    for (auto ty : superTys) {
        if (ty && ty->String() == t2->String()) {
            return true;
        }
    }
    return false;
}

bool Dsl::Subtype(Ty* t1, Ty* t2) const
{
    if (t1 == nullptr || t2 == nullptr) {
        return false;
    }
    return SameType(t1, t2) || IsParentType(t2, t1);
}

std::set<size_t> Dsl::Range(size_t begin, size_t end)
{
    std::set<size_t> res;
    for (size_t i = begin; i < end; ++i) {
        res.emplace(i);
    }
    return res;
}

Node* Dsl::Corresponding(Node* node, const std::map<Node*, Node*>& map)
{
    auto it = map.find(node);
    if (it != map.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

Ty* Dsl::ReturnType(const Node* n)
{
    auto f = dynamic_cast<const FuncDecl*>(n);
    CJC_ASSERT(f != nullptr);
    if (f == nullptr) {
        return nullptr;
    }
    auto ty = f->GetTy().get();
    CJC_ASSERT(ty->IsFunc());
    auto retTy = static_cast<const FuncTy*>(ty)->retTy.get();
    return retTy;
}

bool Dsl::IsNamedParam(const Node* n1)
{
    auto param = dynamic_cast<const FuncParam*>(n1);
    if (param == nullptr) {
        return false;
    }
    return param->isNamedParam;
}

bool Dsl::IsMemberParam(const Node* n1)
{
    auto param = dynamic_cast<const FuncParam*>(n1);
    if (param == nullptr) {
        auto vd = dynamic_cast<const VarDecl*>(n1);
        if (vd) {
            return vd->isMemberParam;
        }
        return false;
    }
    return param->isMemberParam;
}

bool Dsl::HasDefaultValue(const Node* n1)
{
    auto param = dynamic_cast<const FuncParam*>(n1);
    CJC_ASSERT(param != nullptr);
    if (param == nullptr) {
        return false;
    }
    return param->assignment.get() != nullptr || param->desugarDecl.get() != nullptr;
}

std::string Dsl::NameOfParam(const Node* n)
{
    auto param = dynamic_cast<const FuncParam*>(n);
    CJC_ASSERT(param != nullptr);
    if (param == nullptr) {
        return "";
    }
    return param->identifier.GetRawText();
}

std::tuple<bool, bool> Dsl::CheckDepreAnnotation(const Node* x1)
{
    auto v1 = dynamic_cast<const Decl*>(x1);
    CJC_ASSERT(v1 != nullptr);
    bool v1DeprecatedAnno = false;
    bool strict = false;
    for (auto& anno : v1->annotations) {
        if (anno->kind == AnnotationKind::DEPRECATED) {
            v1DeprecatedAnno = true;
            std::string message = "";
            std::string since = "";
            Cangjie::AST::ExtractArgumentsOfDeprecatedAnno(anno, message, since, strict);
        }
    }
    return std::make_tuple(v1DeprecatedAnno, strict);
}

std::tuple<bool, bool> Dsl::CheckCAnnotation(const Node* x1)
{
    auto v1 = dynamic_cast<const Decl*>(x1);
    CJC_ASSERT(v1 != nullptr);
    bool v1CAttr = false;
    bool unsafe = false;
    if (v1->TestAttr(Attribute::C)) {
        v1CAttr = true;
    }
    if (v1->TestAttr(Attribute::UNSAFE)) {
        unsafe = true;
    }
    return std::make_tuple(v1CAttr, unsafe);
}

std::tuple<bool, bool> Dsl::CheckCallingConvAnnotation(const Node* x1)
{
    auto v1 = dynamic_cast<const Decl*>(x1);
    CJC_ASSERT(v1 != nullptr);
    bool cdecl = false;
    bool stdcall = false;
    // Since we are loading from a cjo, we cannot check the CALLING_CONV annotation
    // the CALLING_CONV is not serialised to the cjo file,
    // the attributes of these annotations are serialised
    if (v1->TestAttr(Attribute::C)) {
        cdecl = true;
    }
    if (v1->TestAttr(Attribute::STD_CALL)) {
        stdcall = true;
    }
    return std::make_tuple(cdecl, stdcall);
}

bool Dsl::IsFrozen(const Node* x1)
{
    auto v1 = dynamic_cast<const Decl*>(x1);
    if (!v1) {
        return false;
    }
    return v1->HasAnno(AnnotationKind::FROZEN);
}

bool Dsl::IsVar(const Node* x)
{
    auto v = dynamic_cast<const VarDecl*>(x);
    if (!v) {
        return false;
    }
    return v->isVar;
}

bool Dsl::SameNamelessDecls(const Node* x1, const Node* x2)
{
    auto& x1Decls = (dynamic_cast<const Decl*>(x1))->GetMemberDecls();
    auto& x2Decls = (dynamic_cast<const Decl*>(x2))->GetMemberDecls();
    if (x1Decls.size() != x2Decls.size()) {
        return false;
    }
    for (size_t i = 0; i < x1Decls.size(); ++i) {
        if (!SameNamelessDecl(x1Decls[i].get(), x2Decls[i].get(), ClassDecl(x1))) {
            return false;
        }
    }
    return true;
}

bool Dsl::SameNamelessDecl(const Node* x1, const Node* x2, bool isClass)
{
    if ((IsPublic(x1) && IsPublic(x2)) || (isClass && IsProtected(x1) && IsProtected(x2))) {
        if ((dynamic_cast<const Decl*>(x1))->mangledName == (dynamic_cast<const Decl*>(x2))->mangledName) {
            return true;
        }
        return false;
    }
    if (!SameType(x1->GetTy(), x2->GetTy())) {
        return false;
    }
    if (IsInitialized(x1) && IsInitialized(x2) && VarLetOrConst(x1)) {
        return SameVarValue(x1, x2);
    }
    return true;
}

bool Dsl::SameVarValue(const Node* x1, const Node* x2)
{
    auto v1 = dynamic_cast<const VarDecl*>(x1);
    CJC_ASSERT(v1 != nullptr);
    auto v2 = dynamic_cast<const VarDecl*>(x2);
    CJC_ASSERT(v2 != nullptr);
    if (!Subtype(v1->GetTy(), v2->GetTy()) && !IsParentType(v1->GetTy(), v2->GetTy())) {
        return false;
    }
    if (auto litConstExpr1 = dynamic_cast<LitConstExpr*>(v1->initializer.get().get()); litConstExpr1) {
        if (auto litConstExpr2 = dynamic_cast<LitConstExpr*>(v2->initializer.get().get()); litConstExpr2) {
            if (litConstExpr1->stringValue == litConstExpr2->stringValue) {
                return true;
            } else {
                return false;
            }
        }
    } else if (auto refExpr1 = dynamic_cast<RefExpr*>(v1->initializer.get().get()); refExpr1) {
        if (auto refExpr2 = dynamic_cast<RefExpr*>(v2->initializer.get().get()); refExpr2) {
            if (refExpr1->ref.identifier.GetRawText() == refExpr2->ref.identifier.GetRawText()) {
                return true;
            } else {
                return false;
            }
        }
    } else if (auto callExpr1 = dynamic_cast<CallExpr*>(v1->initializer.get().get()); callExpr1) {
        if (auto callExpr2 = dynamic_cast<CallExpr*>(v2->initializer.get().get()); callExpr2) {
            return SameVarValueCallExpr(callExpr1, callExpr2);
        }
    }
    return true;
}

bool Dsl::SameVarValueCallExpr(const CallExpr* callExpr1, const CallExpr* callExpr2)
{
    if (auto nameExpr1 = dynamic_cast<RefExpr*>(callExpr1->baseFunc.get().get()); nameExpr1) {
        if (auto nameExpr2 = dynamic_cast<RefExpr*>(callExpr2->baseFunc.get().get()); nameExpr2) {
            if (nameExpr1->ref.identifier == nameExpr2->ref.identifier) {
                return true;
            } else {
                return false;
            }
        }
        return true;
    }
    return true;
}

bool Dsl::HasConstOrFrozenInit(Node* node)
{
    auto pDecl = dynamic_cast<Decl*>(node);
    for (auto& d : pDecl->GetMemberDecls()) {
        if (d->TestAnyAttr(Attribute::CONSTRUCTOR) &&
            (IsConst(d.get()) || IsFrozen(d.get()))) {
            return true;
        }
    }
    return false;
}

bool Dsl::IsLet(const Node* x)
{
    auto v = dynamic_cast<const VarDecl*>(x);
    CJC_ASSERT(v != nullptr);
    return !v->isVar && !v->isConst;
}

bool Dsl::IsConst(const Node* x)
{
    auto v = dynamic_cast<const VarDecl*>(x);
    if (v != nullptr) {
        return v->isConst;
    }

    auto f = dynamic_cast<const FuncDecl*>(x);
    if (f != nullptr) {
        return f->isConst;
    }
    return false;
}

bool Dsl::IsClass(const OwnedPtr<Type>& x)
{
    return x && x->GetTy() && x->GetTy()->kind == TypeKind::TYPE_CLASS && Ty::ToString(x->GetTy()) != "Class-Object";
}

bool Dsl::IsInterface(const OwnedPtr<Type>& x)
{
    return x && x->GetTy() && x->GetTy()->kind == TypeKind::TYPE_INTERFACE;
}

size_t Dsl::GetInheritedTypesSize(const Node* x, const std::function<bool(const OwnedPtr<Type>&)> f)
{
    auto decl = dynamic_cast<const InheritableDecl*>(x);
    CJC_ASSERT(decl != nullptr);
    return std::accumulate(decl->inheritedTypes.begin(), decl->inheritedTypes.end(), 0,
        [&](size_t acc, const OwnedPtr<Type>& x) { return acc + (f(x) ? 1 : 0); });
}

std::unordered_set<std::string> Dsl::GetInheritedTypes(
    const Node* x, const std::function<bool(const OwnedPtr<Type>&)> f)
{
    std::unordered_set<std::string> inheritedTypes;
    auto decl = dynamic_cast<const InheritableDecl*>(x);
    CJC_ASSERT(decl != nullptr);
    for (const auto& x : decl->inheritedTypes) {
        if (f(x)) {
            inheritedTypes.emplace(x->ToString());
        }
    }
    return inheritedTypes;
}

std::unordered_set<std::string> Dsl::GetInheritedTypesFromExtendDecl(const Node* ed)
{
    std::unordered_set<std::string> inheritedTypes;
    auto decl = dynamic_cast<const InheritableDecl*>(ed);
    CJC_ASSERT(decl != nullptr);
    if (!TopLevel(decl)) {
        return inheritedTypes;
    }
    for (const auto& x : decl->inheritedTypes) {
        if (IsInterface(x)) {
            inheritedTypes.emplace(x->ToString());
        }
    }
    return inheritedTypes;
}

Ty* Dsl::AliasTargetTy(const Node* n)
{
    if (!n || n->astKind != ASTKind::TYPE_ALIAS_DECL) {
        return nullptr;
    }
    return static_cast<const TypeAliasDecl*>(n)->type->GetTy();
}

Ty* Dsl::TypeOf(const Node* n)
{
    return n->GetTy().get();
}

std::string Dsl::GetMangledName(Decl* x)
{
    if (x->mangledName.empty()) {
        x->mangledName = BaseMangler().Mangle(*x);
    }
    return x->mangledName;
}

bool Dsl::IsClassLikeTy(const Ty* ty)
{
    if (ty == nullptr) {
        return false;
    }
    return ty->IsClassLike();
}

size_t Dsl::BodyHashOf(const Node* n)
{
    auto d = dynamic_cast<const Decl*>(n);
    if (d == nullptr) {
        return 0;
    }
    return d->hash.bodyHash;
}

bool Dsl::IsAtEnd(Node* v, std::set<Node*> declMap, const std::vector<Ptr<Cangjie::AST::Decl>> declList)
{
    auto varsIt = declList.rbegin();
    while (varsIt != declList.rend()) {
        if (*varsIt == v) {
            return true;
        }
        auto it = declMap.find(*varsIt);
        if (it == declMap.end()) {
            return false;
        }
        ++varsIt;
    }
    return false;
}

bool Dsl::IsSameFuncDecl(Node* n1, Node* n2)
{
    if (!Dsl::Func(n1) || !Dsl::Func(n2)) {
        return false;
    }
    return SameFunc(n1, n2);
}

bool Dsl::CheckUnimplementedFuncDecl(const OwnedPtr<Type>& itype, std::vector<Decl*>& decls)
{
    if (!itype) {
        return false;
    }
    auto iTy = static_cast<InterfaceTy*>(itype->GetTy().get());
    if (!iTy || !iTy->decl) {
        return false;
    }
    auto interfaceDecl = iTy->decl;
    for (auto& d : interfaceDecl->body->decls) {
        if (d->TestAttr(Attribute::ABSTRACT)) {
            bool hasImplementedFunc = false;
            std::for_each(decls.begin(), decls.end(), [&](auto& decl) {
                if (IsSameFuncDecl(d.get(), decl)) {
                    hasImplementedFunc = true;
                    return;
                }
                if (IsSamePropDecl(d.get(), decl)) {
                    hasImplementedFunc = true;
                    return;
                }
            });
            if (!hasImplementedFunc) {
                return true;
            }
        } else {
            // Collect func or prop with default implementation.
            decls.emplace_back(d.get());
        }
    }
    for (const auto& it : interfaceDecl->inheritedTypes) {
        if (!Dsl::IsInterface(it)) {
            continue;
        }
        if (CheckUnimplementedFuncDecl(it, decls)) {
            return true;
        }
    }
    return false;
}

bool Dsl::HasUnimplementedInterface(const Node* i1, const Node* i2)
{
    auto decl1 = dynamic_cast<const InheritableDecl*>(i1);
    auto decl2 = dynamic_cast<const InheritableDecl*>(i2);
    if (!decl1 || !decl2) {
        return false;
    }
    if (Dsl::ClassDecl(i1) && !i1->TestAttr(Attribute::ABSTRACT)) {
        return false;
    }
    bool hasUnImplemented = false;
    for (const auto& it2 : decl2->inheritedTypes) {
        if (!Dsl::IsInterface(it2)) {
            continue;
        }
        bool bAdd = true;
        for (const auto& it1 : decl1->inheritedTypes) {
            if (it1->ToString() == it2->ToString()) {
                bAdd = false;
            }
        }
        if (!bAdd) {
            continue;
        }
        std::vector<Decl*> decls;
        if (CheckUnimplementedFuncDecl(it2, decls)) {
            return true;
        }
    }
    return false;
}

bool Dsl::HasOverridingFunc(const OwnedPtr<Type>& itype, Node* fd)
{
    if (!itype) {
        return false;
    }
    if (Dsl::IsInterface(itype)) {
        auto iTy = static_cast<InterfaceTy*>(itype->GetTy().get());
        if (!iTy || !iTy->decl) {
            return false;
        }
        auto id = iTy->decl;
        for (auto& d : id->body->decls) {
            if (d->TestAttr(Attribute::ABSTRACT)) {
                continue;
            }
            if (IsSameFuncDecl(d.get(), fd)) {
                return true;
            }
            if (IsSamePropDecl(d.get(), fd)) {
                return true;
            }
        }
        for (const auto& it : id->inheritedTypes) {
            if (HasOverridingFunc(it, fd)) {
                return true;
            }
        }
        return false;
    }
    if (Dsl::IsClass(itype)) {
        auto cTy = static_cast<ClassTy*>(itype->GetTy().get());
        if (!cTy || !cTy->decl) {
            return false;
        }
        auto cd = cTy->decl;
        for (auto& d : cd->body->decls) {
            if (d->TestAttr(Attribute::ABSTRACT)) {
                continue;
            }
            if (IsSameFuncDecl(d.get(), fd)) {
                return true;
            }
            if (IsSamePropDecl(d.get(), fd)) {
                return true;
            }
        }
        for (const auto& it : cd->inheritedTypes) {
            if (HasOverridingFunc(it, fd)) {
                return true;
            }
        }
    }
    return false;
}

bool Dsl::HasOverridingDecl(Node* n, Node* f)
{
    auto inheritDecl = dynamic_cast<const InheritableDecl*>(n);
    if (!inheritDecl) {
        return false;
    }
    for (const auto& it : inheritDecl->inheritedTypes) {
        if (HasOverridingFunc(it, f)) {
            return true;
        }
    }
    return false;
}

bool Dsl::HasUnimplementedFunc(const OwnedPtr<Type>& itype, Node* fd)
{
    if (!itype) {
        return false;
    }
    if (Dsl::IsInterface(itype)) {
        auto iTy = static_cast<InterfaceTy*>(itype->GetTy().get());
        if (!iTy || !iTy->decl) {
            return false;
        }
        auto interfaceDecl = iTy->decl;
        for (auto& d : interfaceDecl->body->decls) {
            if (IsSameFuncDecl(d.get(), fd) || IsSamePropDecl(d.get(), fd)) {
                return d->TestAttr(Attribute::ABSTRACT);
            }
        }
        for (const auto& it : interfaceDecl->inheritedTypes) {
            if (HasUnimplementedFunc(it, fd)) {
                return true;
            }
        }
        return false;
    }
    if (Dsl::IsClass(itype)) {
        auto cTy = static_cast<ClassTy*>(itype->GetTy().get());
        if (!cTy || !cTy->decl) {
            return false;
        }
        auto classDecl = cTy->decl;
        for (auto& d : classDecl->body->decls) {
            if (IsSameFuncDecl(d.get(), fd) || IsSamePropDecl(d.get(), fd)) {
                return d->TestAttr(Attribute::ABSTRACT);
            }
        }
        for (const auto& it : classDecl->inheritedTypes) {
            if (HasUnimplementedFunc(it, fd)) {
                return true;
            }
        }
    }
    return false;
}

bool Dsl::HasUnimplementedDecl(Node* n1, Node* f1)
{
    auto id = dynamic_cast<const InheritableDecl*>(n1);
    auto fd = dynamic_cast<const Decl*>(f1);
    if (!id || !fd) {
        return false;
    }

    for (const auto& it : id->inheritedTypes) {
        if (HasUnimplementedFunc(it, f1)) {
            return true;
        }
    }
    return false;
}
