// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <sstream>

#include "cangjie/Parse/ASTHasher.h"
#include "cangjie/AST/Clone.h"

#include "cjcompat/Diff/Diff.h"

using namespace Cangjie;

namespace  {
static Ptr<FuncBody> GetFuncBody(const Decl& decl)
{
    switch (decl.astKind) {
        case ASTKind::FUNC_DECL:
            return static_cast<const FuncDecl*>(&decl)->funcBody.get();
        case ASTKind::PRIMARY_CTOR_DECL:
            return static_cast<const PrimaryCtorDecl*>(&decl)->funcBody.get();
        default:
            return nullptr;
    }
}

static void CopyFuncParamNameToFuncArg(Node& cloned)
{
    if (cloned.astKind != AST::ASTKind::CALL_EXPR) {
        return;
    }
    auto ce = static_cast<CallExpr*>(&cloned);
    auto re = static_cast<RefExpr*>(ce->baseFunc.get().get());
    if (!re || !re->ref.target) {
        return;
    }
    auto fd = static_cast<FuncDecl*>(re->ref.target.get());
    if (!fd->funcBody) {
        return;
    }
    std::set<std::string> namedParams;
    auto& params = fd->funcBody->paramLists[0]->params;
    for (auto& param : params) {
        if (param->isNamedParam) {
            namedParams.insert(param->identifier);
        }
    }
    std::string name;
    for (auto& n : namedParams) {
        name += n;
    }
    if (name.empty()) {
        return;
    }
    auto argNum = ce->args.size();
    for (size_t i = 0; i < argNum; i++) {
        if (ce->args[i]->name.Empty() && params[i]->isNamedParam) {
            ce->args[i]->name = name;
        }
    }
}

static size_t HashBody(Decl& decl)
{
    auto funcbody = GetFuncBody(decl);
    if (!funcbody) {
        return ASTHasher::BodyHash(decl, std::make_pair(false, false), false);
    }
    auto fd = MakeOwned<AST::FuncDecl>();
    fd->funcBody = MakeOwned<AST::FuncBody>();
    auto paramList = MakeOwned<FuncParamList>();
    fd->funcBody->paramLists.emplace_back(std::move(paramList));
    fd->funcBody->body = AST::ASTCloner::Clone(funcbody->body.get(), [](const Node& src, Node& cloned) {
        if (cloned.astKind == AST::ASTKind::REF_EXPR) {
            auto re = static_cast<RefExpr*>(&cloned);
            if (re->ref.target) {
                re->ref.target->rawMangleName = "";
                if (re->ref.target->GetTy()->HasGeneric()) {
                    re->ref.target = nullptr;
                }
            }
        }
        CopyFuncParamNameToFuncArg(cloned);
    });
    for (auto& paramlist : funcbody->paramLists) {
        for (auto& param : paramlist->params) {
            if (!param->assignment) {
                continue;
            }
            auto vd = MakeOwned<AST::VarDecl>();
            vd->initializer = AST::ASTCloner::Clone(param->assignment.get());
            param->hash.bodyHash = ASTHasher::BodyHash(*vd, std::make_pair(false, false), false);
        }
    }
    return ASTHasher::BodyHash(*fd, std::make_pair(false, false), false);
}

static void Process(Decl& decl)
{
    if (!decl.IsConst() && !decl.HasAnno(AnnotationKind::FROZEN)) {
        return;
    }
    // If the VarDecl has an initialitzer, ignore the annotations for the bodyHash computation
    if (auto varDecl = dynamic_cast<const VarDecl*>(&decl); varDecl) {
        if (dynamic_cast<LitConstExpr*>(varDecl->initializer.get().get()) ||
            dynamic_cast<RefExpr*>(varDecl->initializer.get().get())) {
            decl.hash.bodyHash = HashBody(decl);
            return;
        }
    }
    // decl is a const of frozen declaration
    decl.hash.bodyHash = HashBody(decl);
    if (decl.astKind == ASTKind::PROP_DECL) {
        auto prop = static_cast<const PropDecl*>(&decl);
        for (auto& getter : prop->getters) {
            getter->hash.bodyHash = HashBody(*getter);
        }
        for (auto& setter : prop->setters) {
            setter->hash.bodyHash = HashBody(*setter);
        }
    }
}

static bool ExtendClassInstanceFunc(const Decl& decl)
{
    if (decl.outerDecl->astKind == ASTKind::EXTEND_DECL) {
        auto ref = static_cast<Cangjie::AST::RefType*>(
            static_cast<Cangjie::AST::ExtendDecl*>(decl.outerDecl.get())->extendedType.get().get());
        if (ref->GetTarget()->astKind == ASTKind::CLASS_DECL) {
            return true;
        }
    }
    return false;
}

static bool ClassProtectedInstanceFunc(const Decl& decl)
{
    if ((decl.astKind == ASTKind::FUNC_DECL || decl.astKind == ASTKind::PROP_DECL) &&
        decl.IsMemberDecl() && decl.TestAttr(Attribute::PROTECTED) &&
        (decl.outerDecl->astKind == ASTKind::CLASS_DECL || ExtendClassInstanceFunc(decl))) {
        return true;
    }
    return false;
}

static void ProcessConstructor(Decl& decl)
{
    if (!decl.IsStructOrClassDecl()) {
        return;
    }
    std::vector<Ptr<AST::Decl>> vars;
    std::vector<Ptr<AST::Decl>> memberparams;
    for (auto& d : decl.GetMemberDecls()) {
        if (d->astKind == ASTKind::VAR_DECL) {
            auto vd = static_cast<VarDecl*>(d.get().get());
            if (vd->isMemberParam) {
                vars.emplace_back(d.get());
            }
        }
        if (d->astKind == ASTKind::FUNC_DECL && d->TestAttr(Attribute::CONSTRUCTOR)) {
            auto fd = static_cast<FuncDecl*>(d.get().get());
            if (!fd->funcBody || fd->funcBody->paramLists.empty()) {
                continue;
            }
            auto& paramList = fd->funcBody->paramLists[0];
            for (auto& param : paramList->params) {
                if (!param->isMemberParam) {
                    continue;
                }
                memberparams.emplace_back(param.get());
            }
        }
    }
    for (auto& param : memberparams) {
        for (auto vd : vars) {
            if (param->identifier == vd->identifier && vd->TestAttr(Attribute::PRIVATE)) {
                param->EnableAttr(Attribute::PRIVATE);
            }
        }
    }
}

static std::string GetTypeName(const Ty* ty)
{
    if (ty->kind != TypeKind::TYPE) {
        return Ty::ToString(ty);
    }
    // If the ty is a TypeAliasTy, get the actual type name of the alias type.
    auto aliasTy = static_cast<const TypeAliasTy*>(ty);
    if (!aliasTy->declPtr || !aliasTy->declPtr->type || !aliasTy->declPtr->type->GetTy()) {
        return Ty::ToString(ty);
    }
    return GetTypeName(aliasTy->declPtr->type->GetTy());
}

static std::string GetExtendTypeName(const AST::ExtendDecl* ed)
{
    if (!ed->extendedType) {
        return "";
    }
    if (ed->extendedType->astKind == AST::ASTKind::PRIMITIVE_TYPE) {
        auto et = static_cast<const AST::PrimitiveType*>(ed->extendedType.get().get());
        return et->str;
    }
    // The extend type is RefType, and may be an alias type.
    if (ed->extendedType->GetTy() && ed->extendedType->GetTy()->kind == TypeKind::TYPE) {
        return GetTypeName(ed->extendedType->GetTy());
    }
    if (ed->extendedType->astKind != AST::ASTKind::REF_TYPE) {
        return "";
    }
    auto et = static_cast<const AST::RefType*>(ed->extendedType.get().get());
    return et->ref.identifier.GetRawText();
}
}

void Diff::CalculateDiff()
{
    const AST::Package& package1 = pkg1;
    const AST::Package& package2 = pkg2;

    std::unordered_map<std::size_t, Ptr<AST::Decl>> declsA;
    std::unordered_map<std::size_t, Ptr<AST::Decl>> declsB;

    for (auto& it : package1.files) {
        CollectFileDecls(it.get(), declsA, true);
    }

    for (auto& it : package2.files) {
        CollectFileDecls(it.get(), declsB, false);
    }

    // Find the deleted declarations in the new AST
    for (const auto& [key, value] : declsA) {
        auto fDecl = declsB.find(key);
        if (fDecl == declsB.end() || value->astKind != fDecl->second->astKind) {
            this->AddDelDecl(value);
        }
    }

    // Find the added and potentially modified declarations in the new AST
    for (const auto& [bKey, bDecl] : declsB) {
        auto aDecl = declsA.find(bKey);
        if (aDecl == declsA.end() || bDecl->astKind != aDecl->second->astKind) {
            this->AddDecl(bDecl);
        } else {
            // Assume all the other decls as potentially modified
            this->SetPotentiallyModifiedElement(aDecl->second, bDecl);
            CollectModifiedMemberDecls(aDecl->second, bDecl);
        }
    }
    CollectImports();
    std::transform(potentiallyModified.begin(), potentiallyModified.end(),
        std::inserter(domPotentiallyModified, domPotentiallyModified.begin()), [](auto& pair) { return pair.first; });

    CollectModifiedMemberOfExtendDecls();
}

/** @brief Prints added, deleted, potentially modified to the standard output */
void Diff::Print() const
{
    std::cout << std::endl << "Deleted decls: " << std::endl;
    if (deleted.empty()) {
        std::cout << "  --none-- " << std::endl;
    } else {
        for (auto decl : deleted) {
            std::cout << "  " << static_cast<Cangjie::AST::Decl*>(decl)->identifier.GetRawText() << std::endl;
        }
    }

    std::cout << std::endl << "Added decls: " << std::endl;
    if (added.empty()) {
        std::cout << "  --none-- " << std::endl;
    } else {
        for (auto decl : added) {
            std::cout << "  " << static_cast<Cangjie::AST::Decl*>(decl)->identifier.GetRawText() << std::endl;
        }
    }

    std::cout << std::endl << "Potentially Modified decls: " << std::endl;
    if (potentiallyModified.empty()) {
        std::cout << "  --none-- " << std::endl;
    } else {
        for (auto decl : potentiallyModified) {
            auto first = static_cast<Cangjie::AST::Decl*>(decl.first);
            auto second = static_cast<Cangjie::AST::Decl*>(decl.second);
            std::cout << "  " << first->identifier << Cangjie::AST::Ty::ToString(first->GetTy()) << " - "
                      << second->identifier << Cangjie::AST::Ty::ToString(second->GetTy()) << std::endl;
        }
    }
    std::cout << std::endl;
}

bool Diff::PropAttrCheck(const Decl* decl) const
{
    auto funcDecl = dynamic_cast<const Cangjie::AST::FuncDecl*>(decl);
    if (funcDecl == nullptr || funcDecl->propDecl == nullptr) {
        return true;
    }
    return ModuleVisible(funcDecl->propDecl);
}

void Diff::HandleRefExprVisibility(const RefExpr* refExpr)
{
    auto pTarget = refExpr->GetTarget();
    if (!pTarget) {
        return;
    }
    if (!pTarget->TestAttr(Attribute::PUBLIC)) {
        InsertVisibleNode(*pTarget.get());
        PropAttrCheck(pTarget.get());
        return;
    }
    if (pTarget->outerDecl &&
        pTarget->outerDecl->TestAttr(Attribute::GLOBAL) &&
        !pTarget->outerDecl->TestAttr(Attribute::PUBLIC)) {
        InsertVisibleNode(*pTarget->outerDecl.get());
    }
}

void Diff::HandleCallExprVisibility(const CallExpr* callExpr)
{
    auto pTarget = callExpr->baseFunc->GetTarget();
    if (!pTarget) {
        return;
    }
    if (pTarget->TestAttr(Attribute::CONSTRUCTOR)) {
        auto outerdecl = pTarget->outerDecl;
        if (outerdecl && !outerdecl->TestAttr(Attribute::PUBLIC)) {
            InsertVisibleNode(*outerdecl);
        }
    }
    if (!pTarget->TestAttr(Attribute::PUBLIC)) {
        auto prop = static_cast<const Cangjie::AST::FuncDecl*>(pTarget.get())->propDecl;
        if (prop == nullptr) {
            InsertVisibleNode(*pTarget.get());
        } else {
            InsertVisibleNode(*prop);
        }
    }
}

void Diff::HandleVarDeclVisibility(const Decl& decl)
{
    auto varDecl = static_cast<const VarDecl*>(&decl);
    if (!varDecl->initializer && varDecl->GetTy()) {
        auto pdecl = Ty::GetDeclPtrOfTy(varDecl->GetTy());
        if (pdecl && (pdecl->astKind == ASTKind::STRUCT_DECL || pdecl->astKind == ASTKind::ENUM_DECL)) {
            InsertVisibleNode(*pdecl);
        }
        return;
    }
    ConstWalker(varDecl->initializer, [this](Ptr<const Node> node) {
        if (node->astKind == ASTKind::REF_EXPR) {
            auto refExpr = static_cast<const RefExpr*>(node.get());
            HandleRefExprVisibility(refExpr);
        } else if (node->astKind == ASTKind::CALL_EXPR) {
            auto callExpr = static_cast<const CallExpr*>(node.get());
            HandleCallExprVisibility(callExpr);
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
}

void Diff::InsertVisibleNode(const Decl& decl)
{
    if (Diff::IsOldNode(&decl)) {
        auto it = visibleNodesOld.find(&decl);
        if (it == visibleNodesOld.end()) {
            Diff::visibleNodesOld.insert(&decl);
            InsertVisibleClassLikeSuperDecl(decl);
            CheckMemberVisibleNode(decl);
        }
    } else {
        auto it = visibleNodesNew.find(&decl);
        if (it == visibleNodesNew.end()) {
            Diff::visibleNodesNew.insert(&decl);
            InsertVisibleClassLikeSuperDecl(decl);
            CheckMemberVisibleNode(decl);
        }
    }
}

void Diff::CheckConstOrFrozenFuncNode(Decl& decl)
{
    if (!decl.IsConst() && !decl.HasAnno(AnnotationKind::FROZEN)) {
        return;
    }
    auto handleVisibility = [this](Ptr<const Node> node) {
        if (node->astKind == ASTKind::REF_EXPR) {
            auto refExpr = static_cast<const RefExpr*>(node.get());
            HandleRefExprVisibility(refExpr);
        } else if (node->astKind == ASTKind::CALL_EXPR) {
            auto callExpr = static_cast<const CallExpr*>(node.get());
            HandleCallExprVisibility(callExpr);
        }
        return VisitAction::WALK_CHILDREN;
    };
    if (decl.astKind == ASTKind::FUNC_DECL) {
        auto funcDecl = static_cast<const FuncDecl*>(&decl);
        ConstWalker(funcDecl->funcBody, handleVisibility).Walk();
    }
    if (decl.astKind == ASTKind::PROP_DECL) {
        auto pd = static_cast<const PropDecl*>(&decl);
        for (auto& get : pd->getters) {
            ConstWalker(get, handleVisibility).Walk();
        }
        for (auto& set : pd->setters) {
            ConstWalker(set, handleVisibility).Walk();
        }
    }
}

void Diff::CheckMemberVisibleNode(const Decl& decl)
{
    for (auto& it : decl.GetMemberDecls()) {
        if (it->TestAttr(Attribute::PUBLIC) || ClassProtectedInstanceFunc(*it)) {
            InsertVisibleNode(*it);
            CheckConstOrFrozenFuncNode(*it);
        }
        if (it->astKind == ASTKind::VAR_DECL) {
            HandleVarDeclVisibility(*it);
        }
    }
}

bool Diff::ExtendExported(std::vector<OwnedPtr<Type>>& inheritedTypes)
{
    for (const auto& it : inheritedTypes) {
        if (it->GetTy() && it->GetTy()->IsInterface()) {
            auto pdecl = Ty::GetDeclOfTy(it->GetTy());
            if (pdecl && pdecl->TestAttr(Attribute::PRIVATE)) {
                return false;
            }
        }
    }
    return true;
}

bool Diff::ExtendTypePublic(Decl& decl)
{
    if (decl.astKind == ASTKind::EXTEND_DECL) {
        auto ed = static_cast<Cangjie::AST::ExtendDecl*>(&decl);
        auto ref = static_cast<Cangjie::AST::RefType*>(ed->extendedType.get().get());
        auto pTarget = ref->GetTarget();
        if (pTarget) {
            if (pTarget->TestAttr(Attribute::PUBLIC)) {
                return true;
            }
            if (pTarget->curFile && ed->curFile &&
                pTarget->curFile->curPackage != ed->curFile->curPackage &&
                ExtendExported(ed->inheritedTypes)) {
                return true;
            }
        } else {
            if (ref->GetTy() && ref->GetTy()->IsPrimitive() && ExtendExported(ed->inheritedTypes)) {
                return true;
            }
        }
    }
    return false;
}

void Diff::CheckAndInsertVisibleNode(Decl& decl)
{
    if (decl.curFile && decl.curFile->curPackage &&
        decl.curFile->curPackage->accessible != AccessLevel::PUBLIC &&
        decl.curFile->curPackage->accessible != AccessLevel::INTERNAL) {
        return;
    }
    if (decl.TestAttr(Attribute::GLOBAL) && decl.TestAttr(Attribute::PUBLIC) &&
        (decl.IsConst() || decl.HasAnno(AnnotationKind::FROZEN))) {
        if (decl.astKind == ASTKind::VAR_DECL) {
            HandleVarDeclVisibility(decl);
        }
        if (decl.astKind == ASTKind::FUNC_DECL) {
            CheckConstOrFrozenFuncNode(decl);
        }
    }
    if (decl.TestAttr(Attribute::PUBLIC) || ExtendTypePublic(decl)) {
        InsertVisibleNode(decl);
    }
}

void Diff::InsertVisibleClassLikeSuperDecl(const Decl& decl)
{
    if (!decl.IsClassLikeDecl()) {
        return;
    }
    auto pClassLikeDecl = static_cast<const ClassLikeDecl*>(&decl);
    std::vector<Ptr<Decl>> superDecls;
    for (auto& it : pClassLikeDecl->inheritedTypes) {
        if (!it->GetTy()) {
            continue;
        }
        if (it->GetTy()->IsClass()) {
            superDecls.emplace_back(static_cast<ClassTy*>(it->GetTy().get())->decl);
        }
        if (it->GetTy()->IsInterface()) {
            superDecls.emplace_back(static_cast<InterfaceTy*>(it->GetTy().get())->decl);
        }
    }
    for (auto& superDecl : superDecls) {
        if (superDecl) {
            InsertVisibleNode(*superDecl);
        }
    }
}

void Diff::CollectDecl(const OwnedPtr<Decl>& decl, std::unordered_map<std::size_t, Ptr<AST::Decl>>& decls)
{
    std::hash<std::string> hash_fn;
    BaseMangler mangler{};
    if (decl->astKind == ASTKind::EXTEND_DECL) {
        if (decl->mangledName.empty()) {
            decl->mangledName = decl->rawMangleName;
        }
        size_t hashValue = hash_fn(decl->mangledName);
        auto found = decls.find(hashValue);
        if (found == decls.end()) {
            Process(*decl);
            decls.emplace(hashValue, decl.get());
        } else {
            auto ed = static_cast<AST::ExtendDecl*>(found->second.get());
            auto& members = decl->GetMemberDecls();
            ed->members.insert(ed->members.end(),
                std::make_move_iterator(members.begin()), std::make_move_iterator(members.end()));
            members.clear();
        }
    } else {
        if (decl->mangledName.empty()) {
            decl->mangledName = mangler.Mangle(*decl);
        }
        size_t hashValue = hash_fn(decl->mangledName);
        auto found = decls.find(hashValue);
        if (found == decls.end()) {
            Process(*decl);
            decls.emplace(hashValue, decl.get());
        }
    }
}

void Diff::CollectFileDecls(
    Ptr<const AST::File> file, std::unordered_map<std::size_t, Ptr<AST::Decl>>& decls, bool isOld)
{
    auto assignWalkerID = [&](Ptr<Node> curNode) -> VisitAction {
        curNode->visitedByWalkerID = oldWalkerID;
        return VisitAction::WALK_CHILDREN;
    };
    for (auto& it : file->decls) {
        ProcessConstructor(*it);
        CheckAndInsertVisibleNode(*it);
        CollectDecl(it, decls);
        if (isOld) {
            Walker(it.get(), assignWalkerID).Walk();
        }
    }
    for (auto& it : file->exportedInternalDecls) {
        CheckAndInsertVisibleNode(*it);
        CollectDecl(it, decls);
        if (isOld) {
            Walker(it.get(), assignWalkerID).Walk();
        }
    }
}

void Diff::CollectMemberDecls(Ptr<AST::Decl> decl, std::unordered_map<std::size_t, Ptr<AST::Decl>>& members)
{
    std::hash<std::string> hash_fn;
    for (auto& it : decl->GetMemberDecls()) {
        // for genric type, the mangledName is empty.
        if (it->mangledName.empty()) {
            if (decl->astKind == ASTKind::EXTEND_DECL) {
                it->mangledName = it->rawMangleName;
            } else {
                it->mangledName = BaseMangler().Mangle(*it);
            }
        }
        size_t hashValue = hash_fn(it->mangledName);
        auto found = members.find(hashValue);
        if (found == members.end()) {
            Process(*it);
            members.emplace(hashValue, it.get());
        }
    }
    if (decl->astKind == ASTKind::ENUM_DECL) {
        for (auto& it : static_cast<Cangjie::AST::EnumDecl*>(decl.get())->constructors) {
            if (it->mangledName.empty()) {
                it->mangledName = BaseMangler().Mangle(*it);
            }
            size_t hashValue = hash_fn(it->mangledName);
            auto found = members.find(hashValue);
            if (found == members.end()) {
                members.emplace(hashValue, it.get());
            }
        }
    }
}

void Diff::CollectModifiedMemberDecls(Ptr<AST::Decl> declA, Ptr<AST::Decl> declB)
{
    std::unordered_map<std::size_t, Ptr<AST::Decl>> membersA;
    std::unordered_map<std::size_t, Ptr<AST::Decl>> membersB;
    CollectMemberDecls(declA, membersA);
    CollectMemberDecls(declB, membersB);

    auto& memberDiff = potentiallyMemberModified[declA.get()];
    // Find the deleted member declarations
    for (const auto& [key, value] : membersA) {
        auto fDecl = membersB.find(key);
        if (fDecl == membersB.end()) {
            memberDiff.deleted.insert(value);
        }
    }
    // Find the added member declarations
    for (const auto& [bKey, bDecl] : membersB) {
        auto aDecl = membersA.find(bKey);
        if (aDecl == membersA.end()) {
            memberDiff.added.insert(bDecl);
            continue;
        }
        // Find the modified member declarations
        memberDiff.potentiallyModified[aDecl->second] = bDecl;
        std::transform(memberDiff.potentiallyModified.begin(), memberDiff.potentiallyModified.end(),
            std::inserter(memberDiff.domPotentiallyModified, memberDiff.domPotentiallyModified.begin()),
            [](auto& pair) { return pair.first; });
    }
}

bool Diff::SameExtend(Node* n1, Node* n2) const
{
    auto ed1 = dynamic_cast<const AST::ExtendDecl*>(n1);
    auto ed2 = dynamic_cast<const AST::ExtendDecl*>(n2);
    if (!ed1 || !ed2) {
        return false;
    }
    auto etn1 = GetExtendTypeName(ed1);
    auto etn2 = GetExtendTypeName(ed2);
    if (etn1.empty() || etn2.empty() || etn1 != etn2) {
        return false;
    }
    return true;
}

void Diff::CollectModifiedMemberOfExtendDecls()
{
    for (auto e1 : GetDeleted()) {
        if (e1->astKind != ASTKind::EXTEND_DECL) {
            continue;
        }
        for (auto e2 : GetAdded()) {
            if (e2->astKind != ASTKind::EXTEND_DECL || !SameExtend(e1, e2)) {
                continue;
            }
            Ptr<AST::Decl> declA = static_cast<AST::Decl*>(e1);
            Ptr<AST::Decl> declB = static_cast<AST::Decl*>(e2);
            CollectModifiedMemberDecls(declA, declB);
        }
    }
}

std::string Diff::PrettyPrintNode(Ptr<const Cangjie::AST::Node> node)
{
    std::string res = "";
    Cangjie::Meta::match (*node)(
        [&res](const AST::GenericParamDecl& typeDecl) {
            res = "GenericParamDecl " + typeDecl.identifier.GetRawText();
        },
        [&res](const AST::FuncParam& param) { res = "FuncParam " + param.identifier.GetRawText(); },
        [&res](const AST::MacroExpandParam& macroExpand) {
            res = "MacroExpandParam " + macroExpand.identifier.GetRawText();
        },
        [&res](const AST::FuncParamList& paramList) { res = "FuncParamList "; },
        [&res](const AST::MainDecl& mainDecl) { res = "MainDecl " + mainDecl.identifier.GetRawText(); },
        [&res](const AST::FuncDecl& funcDecl) { res = "FuncDecl " + funcDecl.identifier.GetRawText(); },
        [&res](const AST::MacroDecl& macroDecl) { res = "MacroDecl " + macroDecl.identifier.GetRawText(); },
        [&res](const AST::FuncBody& body) { res = "FuncBody "; },
        [&res](const AST::PropDecl& propDecl) { res = "PropDecl " + propDecl.identifier.GetRawText(); },
        [&res](const AST::MacroExpandDecl& macroExpand) {
            res = "MacroExpandDecl " + macroExpand.identifier.GetRawText();
        },
        [&res](const AST::VarWithPatternDecl& varWithPatternDecl) {
            res = "VarWithPatternDecl " + varWithPatternDecl.identifier.GetRawText();
        },
        [&res](const AST::VarDecl& varDecl) { res = "VarDecl " + varDecl.identifier.GetRawText(); },
        [&res](const AST::TypeAliasDecl& alias) { res = "TypeAliasDecl " + alias.identifier.GetRawText(); },
        [&res](const AST::ClassDecl& classDecl) { res = "ClassDecl " + classDecl.identifier.GetRawText(); },
        [&res](const AST::InterfaceDecl& interfaceDecl) {
            res = "InterfaceDecl " + interfaceDecl.identifier.GetRawText();
        },
        [&res](const AST::EnumDecl& enumDecl) { res = "EnumDecl " + enumDecl.identifier.GetRawText(); },
        [&res](const AST::StructDecl& decl) { res = "StructDecl " + decl.identifier.GetRawText(); },
        [&res](const AST::ExtendDecl& ed) { res = "ExtendDecl " + ed.identifier.GetRawText(); },
        [&res](const AST::PrimaryCtorDecl& decl) { res = "PrimaryCtorDecl " + decl.identifier.GetRawText(); },
        [&node]() {
            const auto n = *node.get();
            std::cout << typeid(n).name() << " " << std::endl;
        });
    return res;
}

Node* Diff::GetPotentiallyModifiedElement(Node* key)
{
    auto it = potentiallyModified.find(key);
    if (it != potentiallyModified.end()) {
        return it->second;
    }
    return nullptr;
}

void Diff::SetPotentiallyModifiedElement(Node* key, Node* value)
{
    potentiallyModified[key] = value;
}

bool Diff::HasPotentiallyModifiedElement(Decl* key)
{
    return potentiallyModified.find(key) != potentiallyModified.end();
}

void Diff::AddDecl(Node* element)
{
    added.insert(element);
}

void Diff::AddDelDecl(Node* element)
{
    deleted.insert(element);
}

std::set<Node*> Diff::GetDeleted() const
{
    return deleted;
}

std::set<Node*> Diff::GetAdded() const
{
    return added;
}

std::set<Node*> Diff::GetDomPotentiallyModified() const
{
    return domPotentiallyModified;
}

std::string Diff::ToString()
{
    std::string res = "Diff report:\n";
    res += "Potentially Modified decls: \n";
    for (const auto& [bKey, bDecl] : potentiallyModified) {
        res += " - " + PrettyPrintNode(bDecl) + "\n";
    }
    res += "Added decls: \n";
    for (auto& it : added) {
        res += " - " + PrettyPrintNode(it) + "\n";
    }
    res += "Deleted decls: \n";
    for (auto& it : deleted) {
        res += " - " + PrettyPrintNode(it) + "\n";
    }
    return res;
}

const std::map<Cangjie::AST::Node*, Cangjie::AST::Node*>& Diff::PotentiallyModified() const
{
    return potentiallyModified;
}

std::set<Node*> Diff::GetMemberDeleted(Node* key) const
{
    auto it = potentiallyMemberModified.find(key);
    if (it != potentiallyMemberModified.end()) {
        return it->second.deleted;
    }
    return {};
}

std::set<Node*> Diff::GetMemberAdded(Node* key) const
{
    auto it = potentiallyMemberModified.find(key);
    if (it != potentiallyMemberModified.end()) {
        return it->second.added;
    }
    return {};
}

const std::map<AST::Node*, AST::Node*> Diff::GetPotentiallyMemberModified(Node* key) const
{
    auto it = potentiallyMemberModified.find(key);
    if (it != potentiallyMemberModified.end()) {
        return it->second.potentiallyModified;
    }
    return {};
}

std::set<Node*> Diff::GetDomPotentiallyMemberModified(Node* key) const
{
    auto it = potentiallyMemberModified.find(key);
    if (it != potentiallyMemberModified.end()) {
        return it->second.domPotentiallyModified;
    }
    return {};
}

void Diff::CollectImports()
{
    std::unordered_map<std::string, Ptr<AST::ImportSpec>> importsA;
    std::unordered_map<std::string, Ptr<AST::ImportSpec>> importsB;

    for (auto& file : pkg1.files) {
        for (auto& import : file->imports) {
            std::stringstream fullPath;
            for (auto& x : import->content.prefixPaths) {
                fullPath << x << "/";
            }
            fullPath << import->content.identifier;
            importsA.emplace(fullPath.str(), import.get());
        }
    }

    for (auto& file : pkg2.files) {
        for (auto& import : file->imports) {
            std::stringstream fullPath;
            for (auto& x : import->content.prefixPaths) {
                fullPath << x << "/";
            }
            fullPath << import->content.identifier;
            importsB.emplace(fullPath.str(), import.get());
        }
    }

    // Find the deleted imports in the new AST
    for (const auto& [aKey, aImport] : importsA) {
        auto bIt = importsB.find(aKey);
        if (bIt == importsB.end()) {
            this->AddDelDecl(aImport);
        }
    }

    // Find the added and potentially modified imports in the new AST
    for (const auto& [bKey, bImport] : importsB) {
        auto aIt = importsA.find(bKey);
        if (aIt == importsA.end()) {
            this->AddDecl(bImport);
        } else {
            // Assume all the other imports as potentially modified
            this->SetPotentiallyModifiedElement(aIt->second, bImport);
        }
    }
}

bool Diff::ModuleVisible(const Node* key) const
{
    if (IsOldNode(key)) {
        auto visibleNode = visibleNodesOld.find(key);
        return visibleNode != visibleNodesOld.end();
    }
    auto visibleNode = visibleNodesNew.find(key);
    return visibleNode != visibleNodesNew.end();
}