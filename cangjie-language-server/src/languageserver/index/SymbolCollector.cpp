// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <queue>
#include "../CompilerCangjieProject.h"
#include "MemIndex.h"
#include "CjdIndex.h"
#include "SymbolCollector.h"

namespace {
using namespace Cangjie;
using namespace AST;
Ptr<Decl> GetRealDecl(Ptr<Decl> decl)
{
    if (auto fd = DynamicCast<FuncDecl *>(decl); fd && fd->propDecl) {
        return fd->propDecl;
    }
    return decl;
}

TypeSubst GenerateTypeMapping(Ptr<const Generic> generic, const std::vector<Ptr<Ty>> &typeArgs)
{
    TypeSubst mapping;
    if (!generic || generic->typeParameters.size() != typeArgs.size()) {
        return mapping;
    }
    for (size_t i = 0; i < typeArgs.size(); ++i) {
        if (Ty::IsTyCorrect(generic->typeParameters[i]->ty) && Ty::IsTyCorrect(typeArgs[i])) {
            if (auto genParam = DynamicCast<GenericsTy*>(generic->typeParameters[i]->ty)) {
                mapping[genParam] = typeArgs[i];
            }
        }
    }
    return mapping;
}

inline bool IsFitBaseMemberOverriddenCondition(const Decl &parent, const Decl &child)
{
    // Members must have same astKind, identifier, static status, generic status.
    return parent.astKind == child.astKind && parent.identifier == child.identifier &&
           !child.TestAnyAttr(Attribute::CONSTRUCTOR, Attribute::ENUM_CONSTRUCTOR) &&
           (parent.TestAttr(Attribute::STATIC) == child.TestAttr(Attribute::STATIC)) &&
           (parent.TestAttr(Attribute::GENERIC) == child.TestAttr(Attribute::GENERIC));
}

void GetAliasInImportMulti(std::unordered_map<std::string, std::string> aliasMap,
                           const std::vector<ImportContent>& contents)
{
    for (const auto &content : contents) {
        if (content.kind == ImportKind::IMPORT_ALIAS) {
            (void)aliasMap.insert_or_assign(content.identifier, content.aliasName);
        }
    }
}

inline bool NeedToObtainCjdDeclPos(bool isCjoPkg)
{
    return ark::MessageHeaderEndOfLine::GetIsDeveco() && ark::lsp::CjdIndexer::GetInstance() != nullptr && isCjoPkg;
}

ark::lsp::Modifier GetDeclModifier(const Decl &decl)
{
    if (decl.TestAttr(Attribute::PUBLIC)) {
        return ark::lsp::Modifier::PUBLIC;
    } else if (decl.TestAttr(Attribute::PROTECTED)) {
        return ark::lsp::Modifier::PROTECTED;
    } else if (decl.TestAttr(Attribute::INTERNAL)) {
        return ark::lsp::Modifier::INTERNAL;
    } else if (decl.TestAttr(Attribute::PRIVATE)) {
        return ark::lsp::Modifier::PRIVATE;
    }
    return ark::lsp::Modifier::UNDEFINED;
}

bool IsExportExtendSuper(Ptr<ClassLikeDecl> decl)
{
    if (!decl || decl->TestAttr(Attribute::PRIVATE)) {
        return false;
    }
    if (decl->TestAnyAttr(Attribute::PUBLIC, Attribute::PROTECTED, Attribute::INTERNAL)) {
        return true;
    }
    // When the decl is `extend A<B>`, B may be decl without modifiers such as GenericParamDecl, BuiltinDecl.
    // In this case, they must be exported for extend's extendType checking.
    return true;
}

bool CheckExtendExported(const ExtendDecl* decl)
{
    auto extendedDecl = Ty::GetDeclPtrOfTy<InheritableDecl>(decl->ty);
    bool isInSamePkg = extendedDecl && extendedDecl->fullPackageName == decl->fullPackageName;
    auto isUpperBoundExport = [decl]() {
        bool isUpperboundAllExported = true;
        for (auto& tp : decl->generic->genericConstraints) {
            if (!tp) { return false; }
            for (auto& up : tp->upperBounds) {
                if (!up || up->GetTarget() == nullptr) {
                    continue;
                }
                isUpperboundAllExported = up->GetTarget()->IsExportedDecl() && isUpperboundAllExported;
            }
        }
        return isUpperboundAllExported;
    };
    // Direct Extensions Check:
    if (decl->inheritedTypes.empty()) {
        // Rule 1: In `package std.core`, direct extensions of types visible outside the package are exported.
        if (decl->fullPackageName == "std.core") {
            return true;
        }
        if (isInSamePkg) {
            // Rule 2: When direct extensions are defined in the same `package` as the extended type, whether the
            // extension is exported is determined by the lowest access level of the type used in the extended type and
            // the generic constraints (if any).
            if (!decl->generic) {
                return extendedDecl->IsExportedDecl();
            }
            return extendedDecl->IsExportedDecl() && isUpperBoundExport();
        }
        // Rule 3: When the direct extension is in a different `package` from the declaration of the type being
        // extended, the extension is never exported and can only be used in the current `package`.
        return false;
    }
    // Interface Extensions Check:
    if (isInSamePkg) {
        // Rule 1: When the interface extension and the extended type are in the same `package`, the extension is
        // exported together with the extended type and is not affected by the access level of the interface type.
        return extendedDecl->IsExportedDecl();
    }
    // Rule 2: When an interface extension is in a different `package` from the type being extended, whether the
    // interface extension is exported is determined by the smallest access level of the type used in the
    // interface type and the generic constraints (if any).
    bool isInterfaceAllExported = false;
    for (auto& inhertType : decl->inheritedTypes) {
        if (!inhertType || !inhertType->GetTarget()) {
            continue;
        }
        auto target = inhertType->GetTarget();
        if (auto decl = DynamicCast<ClassLikeDecl>(target)) {
            if (IsExportExtendSuper(decl)) {
                isInterfaceAllExported = true;
                break;
            }
        }
    }
    if (!decl->generic) {
        return isInterfaceAllExported;
    }
    return isInterfaceAllExported && isUpperBoundExport();
}

bool IsExportedExtendDecl(const ExtendDecl* decl)
{
    if (!decl) {
        return false;
    }
    // ExtendedType Check (Direct and Interface Extensions): If B in extend A<B> isn't exported, the extendDecl should
    // not be exported. For imported decl, extendedType may be nullptr and not ready.
    if (decl->extendedType != nullptr) {
        for (auto& it : decl->extendedType->GetTypeArgs()) {
            if (it && it->GetTarget() && !it->GetTarget()->IsExportedDecl()) {
                return false;
            }
        }
    }
    return CheckExtendExported(decl);
}
} // namespace

namespace ark::lsp {
const std::unordered_set<ASTKind> G_IGNORE_KINDS{ASTKind::MAIN_DECL, ASTKind::MACRO_DECL,
                                                 ASTKind::VAR_WITH_PATTERN_DECL,
                                                 ASTKind::PRIMARY_CTOR_DECL, ASTKind::INVALID_DECL};

void SymbolCollector::Preamble(const Package &package)
{
    for (const auto &file : package.files) {
        for (const auto &import : file->imports) {
            if (import->IsImportAlias()) {
                (void)aliasMap.insert_or_assign(import->content.identifier, import->content.aliasName.Val());
            }
            if (import->IsImportMulti()) {
                GetAliasInImportMulti(aliasMap, import->content.items);
            }
        }
    }
}

bool SymbolCollector::ShouldPassInCjdIndexing(Ptr<Node> node)
{
    return CjdIndexer::GetInstance() && CjdIndexer::GetInstance()->GetRunningState() && DynamicCast<FuncDecl *>(node);
}

void SymbolCollector::Build(const Package &package)
{
    Preamble(package);
    std::unordered_set<Ptr<InheritableDecl>> inheritableDecls;
    (void)scopes.emplace_back(&package, package.fullPackageName + ":");
    for (auto &file : package.files) {
        auto filePath = file->curFile->filePath;
        auto collectPre = [this, &inheritableDecls, &filePath](auto node) {
            if (auto invocation = node->GetConstInvocation()) {
                CreateMacroRef(*node, *invocation);
            }
            if (!Ty::IsTyCorrect(node->ty)) {
                if (!ShouldPassInCjdIndexing(node)) {
                    return VisitAction::WALK_CHILDREN;
                }
            }
            if (node->astKind == ASTKind::PRIMARY_CTOR_DECL ||
                node->TestAnyAttr(Attribute::MACRO_INVOKE_FUNC, Attribute::IN_CORE)) {
                return VisitAction::SKIP_CHILDREN;
            }
            if (auto fd = DynamicCast<FuncDecl *>(node); fd && fd->propDecl) {
                return VisitAction::WALK_CHILDREN;
            } else if (auto id = DynamicCast<InheritableDecl *>(node)) {
                (void)inheritableDecls.emplace(id);
            }
            if (Utils::In(node->astKind, G_IGNORE_KINDS)) {
                return VisitAction::WALK_CHILDREN;
            }
            if (auto decl = DynamicCast<Decl *>(node)) {
                CreateBaseOrExtendSymbol(*decl, filePath);
                UpdateScope(*decl);
            } else if (auto ref = DynamicCast<NameReferenceExpr *>(node)) {
                CreateRef(*ref, filePath);
            } else if (auto ce = DynamicCast<CallExpr *>(node); ce && !ce->desugarExpr) {
                CreateNamedArgRef(*ce);
            } else if (auto type = DynamicCast<Type *>(node)) {
                CreateTypeRef(*type, filePath);
            }
            return VisitAction::WALK_CHILDREN;
        };

        auto collectPost = [this](auto node) {
            RestoreScope(*node);
            return VisitAction::WALK_CHILDREN;
        };

        Walker(file.get(), collectPre, collectPost).Walk();

        // Some desugar node information is stored in trashBin.
        for (auto &it : file->trashBin) {
            Walker(it.get(), collectPre, collectPost).Walk();
        }

        for (auto &it : file->originalMacroCallNodes) {
            auto invocation = it->GetConstInvocation();
            if (!invocation) {
                continue;
            }
            CreateMacroRef(*it, *invocation);
            Walker(invocation->decl.get(), [this](auto node) {
                if (auto i = node->GetConstInvocation()) {
                    CreateMacroRef(*node, *i);
                    return VisitAction::WALK_CHILDREN;
                }
                return VisitAction::WALK_CHILDREN;
            }).Walk();
        }
    }
    scopes.pop_back();
    CollectRelations(inheritableDecls);
}

void SymbolCollector::CreateBaseOrExtendSymbol(const Decl &decl, const std::string &filePath)
{
    auto inSupportBaseSym = IsGlobalOrMemberOrItsParam(decl) || IsLocalFuncOrLambda(decl);
    if (inSupportBaseSym) {
        CreateBaseSymbol(decl, filePath);
    }  else if (IsExtendDecl(decl)) {
        CreateExtend(decl, filePath);
    }
}

void SymbolCollector::CreateBaseSymbol(const Decl &decl, const std::string &filePath)
{
    if (!decl.curFile) {
        return;
    }
    CJC_ASSERT(!scopes.empty());
    std::string curScope;
    for (auto [_, scope] : scopes) {
        curScope += scope;
    }
    curScope.pop_back(); // Pop back last delimiter char.

    auto &identifier =
        decl.TestAttr(Attribute::PRIMARY_CONSTRUCTOR) ? decl.identifierForLsp : decl.identifier;
    SymbolLocation loc;
    if (decl.TestAttr(Attribute::IMPLICIT_ADD, Attribute::CONSTRUCTOR)) {
        loc = {.begin = {0, 0}, .end = {0, 0}, .fileUri = decl.curFile->filePath};
    } else {
        auto identifierPos = decl.GetIdentifierPos();
        auto uri = decl.curFile->filePath;
        // decl in .macroCall file
        if (identifierPos.fileID != decl.begin.fileID) {
            uri = decl.curFile->macroCallFilePath;
        }
        loc = {identifierPos, identifierPos + CountUnicodeCharacters(identifier), uri};
    }
    SymbolLocation curMacroCall = {.begin = {0, 0}, .end = {0, 0}, .fileUri = ""};
    auto curMacroCallNode = decl.curMacroCall;
    if (curMacroCallNode && curMacroCallNode->curFile) {
        curMacroCall = {curMacroCallNode->begin, curMacroCallNode->end, curMacroCallNode->curFile->filePath};
    }
    SymbolLocation declaration = {.begin = {0, 0}, .end = {0, 0}, .fileUri = ""};
    if (NeedToObtainCjdDeclPos(isCjoPkg)) {
        declaration = CjdIndexer::GetInstance()->GetSymbolDeclaration(GetDeclSymbolID(decl), decl.fullPackageName);
    }
    Symbol declSym{.id = GetDeclSymbolID(decl),
                   .name = identifier,
                   .scope = curScope,
                   .location = loc,
                   .declaration = declaration,
                   .kind = decl.astKind,
                   .signature = ItemResolverUtil::ResolveSignatureByNode(decl),
                   .returnType = GetTypeString(decl)};
    // config local lambda variables kind for callHierarchy
    bool isLocalLambda = declSym.kind == ASTKind::VAR_DECL && isLambda(decl);
    if (isLocalLambda) {
        declSym.kind = ASTKind::LAMBDA_EXPR;
    }
    declSym.modifier = GetDeclModifier(decl);
    declSym.isCjoSym = isCjoPkg;
    declSym.insertText = ItemResolverUtil::ResolveInsertByNode(decl);
    auto funcDecl = dynamic_cast<const FuncDecl*>(&decl);
    if (decl.TestAttr(Cangjie::AST::Attribute::MACRO_FUNC) && funcDecl && funcDecl->funcBody) {
        declSym.signature = decl.identifier;
        ItemResolverUtil::ResolveMacroParams(declSym.signature, funcDecl->funcBody->paramLists);
    }
    declSym.curMacroCall = curMacroCall;
    if (decl.HasAnno(AnnotationKind::DEPRECATED)) {
        declSym.isDeprecated = true;
    }
    if (!declSym.isCjoSym && decl.curFile->curPackage) {
        declSym.curModule = SplitFullPackage(decl.curFile->curPackage->fullPackageName).first;
    }
    if (auto vd = DynamicCast<const VarDecl *>(&decl)) {
        declSym.isMemberParam = vd->isMemberParam;
    }
    if (!loc.IsZeroLoc()) {
        UpdatePos(declSym.location, decl, filePath);
    }
    (void)pkgSymsMap.emplace_back(declSym);

    Ref refInfo{.location = loc, .kind = RefKind::DEFINITION, .container = GetContextID(), .isCjoRef = isCjoPkg};
    if (!loc.IsZeroLoc()) {
        UpdatePos(refInfo.location, decl, filePath);
    }
    (void)symbolRefMap[declSym.id].emplace_back(refInfo);
}

struct ExtendInfo {
    SymbolID id;
    std::string name;
    ark::lsp::Modifier modifier;
    std::string interfaceName;
};

void SymbolCollector::CreateExtend(const Decl &decl, const std::string &filePath)
{
    auto extendDecl = DynamicCast<ExtendDecl *>(&decl);
    bool isInvalidExtend = !extendDecl || !IsExportedExtendDecl(extendDecl) || !extendDecl->extendedType ||
        extendDecl->inheritedTypes.empty();
    if (isInvalidExtend) {
        return;
    }
    auto target = extendDecl->extendedType->GetTarget();
    if (!target || (target->ty && target->ty->HasGeneric())) {
        return;
    }
    std::string targetName = ItemResolverUtil::ResolveSignatureByNode(*target);
    SymbolID symbolID = GetDeclSymbolID(*target);
    auto fullPackageName = extendDecl->fullPackageName;
    std::vector<ExtendInfo> extendVec;
    std::map<std::string, ExtendInfo> extendInfoMap;
    for (auto &member : extendDecl->members) {
        bool isExported = member->IsExportedDecl();
        if (!isExported) {
            continue;
        }
        std::string signature = ItemResolverUtil::ResolveSignatureByNode(*member);
        auto modifier = GetDeclModifier(*member);
        auto extendSymbolID = GetDeclSymbolID(*member);
        ExtendItem extendItem = {.id = extendSymbolID};
        extendVec.push_back({.id=extendSymbolID, .name=signature});
        extendInfoMap.insert_or_assign(signature, extendVec.back());
    }
    std::function<void(const InheritableDecl&, std::vector<Ptr<Decl>>&)> collectInheritMember =
        [&collectInheritMember, &extendInfoMap](const InheritableDecl& id,
                                                std::vector<Ptr<Decl>>& members) {
        for (auto& super : id.inheritedTypes) {
            auto targetDecl = DynamicCast<InheritableDecl>(super->GetTarget());
            if (targetDecl == nullptr) {
                continue;
            }
            collectInheritMember(*targetDecl, members);
        }
        if (id.astKind != ASTKind::INTERFACE_DECL) {
            return;
        }
        std::string interfaceName = ItemResolverUtil::ResolveSignatureByNode(id);
        auto modifier = GetDeclModifier(id);
        auto& inherMembers = id.GetMemberDecls();
        std::for_each(inherMembers.begin(), inherMembers.end(),
            [&members, &interfaceName, &modifier, &extendInfoMap](auto& m) {
            std::string name = ItemResolverUtil::ResolveSignatureByNode(*m);
            if (extendInfoMap.find(name) == extendInfoMap.end() && !m->TestAttr(Attribute::ABSTRACT)) {
                members.emplace_back(m);
            } else {
                extendInfoMap[name].interfaceName = interfaceName;
                extendInfoMap[name].modifier = modifier;
            }
        });
    };
    // get method with interface modifier
    for (const auto& types : extendDecl->inheritedTypes) {
        auto targetDecl = DynamicCast<InheritableDecl>(types->GetTarget());
        if (targetDecl == nullptr) {
            continue;
        }
        if (targetDecl->astKind != ASTKind::INTERFACE_DECL) {
            return;
        }
        auto interfaceModifier = GetDeclModifier(*targetDecl);
        auto extendSymbolID = GetDeclSymbolID(*targetDecl);
        std::vector<Ptr<Decl>> members;
        collectInheritMember(*targetDecl, members);
        std::string interfaceName = ItemResolverUtil::ResolveSignatureByNode(*targetDecl);
        for (const auto& member : members) {
            std::string signature = ItemResolverUtil::ResolveSignatureByNode(*member);
            auto extendSymbolID = GetDeclSymbolID(*member);
            ExtendInfo info = {.id = extendSymbolID, .name = signature,
                               .modifier = interfaceModifier, .interfaceName = interfaceName};
            extendInfoMap.insert_or_assign(signature, info);
        }
    }
    for (const auto& info : extendInfoMap) {
        ExtendItem extendItem = {.id = info.second.id,
                                 .modifier = info.second.modifier,
                                 .interfaceName = info.second.interfaceName};
        (void)symbolExtendMap[symbolID].emplace_back(extendItem);
    }
}

void SymbolCollector::CreateRef(const NameReferenceExpr &ref, const std::string &filePath)
{
    if (ref.TestAttr(Attribute::IMPLICIT_ADD)) {
        return;
    }
    auto refExpr = DynamicCast<const RefExpr *>(&ref);
    if (refExpr) {
        if (refExpr->isThis) {
            return;
        }
    }
    auto target = ref.aliasTarget ? ref.aliasTarget : GetRealDecl(ref.GetTarget());
    if (target == nullptr || (!IsGlobalOrMemberOrItsParam(*target) && !IsLocalFuncOrLambda(*target))) {
        return;
    }
    if (!ref.curFile) {
        return;
    }
    auto ma = DynamicCast<const MemberAccess *>(&ref);
    if (ma && ma->field.Begin().IsZero()) {
        return;
    }
    auto identifier = target->TestAttr(Attribute::CONSTRUCTOR) ? target->outerDecl->identifier
                                                               : target->identifier;
    if (aliasMap.find(identifier) != aliasMap.end()) {
        identifier = aliasMap[identifier];
    }
    auto offset = (identifier == "[]" || identifier == "()") ? 1 : CountUnicodeCharacters(identifier);
    auto begin = ma ? ma->GetFieldPos() : (refExpr ? refExpr->GetIdentifierPos() : ref.GetBegin());
    if (begin == INVALID_POSITION) {
        return;
    }
    auto end = begin + offset;
    auto uri = ref.curFile->filePath;
    if (begin.fileID != ref.begin.fileID) {
        uri = ref.curFile->macroCallFilePath;
    }
    if (uri.empty()) {
        return;
    }
    SymbolLocation loc{begin, end, uri};
    Ref refInfo{.location = loc, .kind = RefKind::REFERENCE, .container = GetContextID()};
    UpdatePos(refInfo.location, ref, filePath);
    (void)symbolRefMap[GetDeclSymbolID(*target)].emplace_back(refInfo);
}

void SymbolCollector::CreateTypeRef(const Type &type, const std::string &filePath)
{
    auto target = type.GetTarget();
    if (target == nullptr || !target->TestAttr(Attribute::GLOBAL)) {
        return;
    }
    if (!type.curFile) {
        return;
    }
    auto qt = DynamicCast<const QualifiedType *>(&type);
    if (qt && qt->field.Begin().IsZero()) {
        return;
    }
    auto begin = qt ? qt->GetFieldPos() : type.GetBegin();
    if (!type.typePos.IsZero()) {
        begin = type.typePos;
    }
    auto end = qt ? begin + CountUnicodeCharacters(qt->field) : begin + CountUnicodeCharacters(target->identifier);
    SymbolLocation loc{begin, end, type.curFile->filePath};
    Ref refInfo{.location = loc, .kind = RefKind::REFERENCE, .container = GetContextID()};
    if (type.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }
    UpdatePos(refInfo.location, type, filePath);
    (void)symbolRefMap[GetDeclSymbolID(*target)].emplace_back(refInfo);
}

void SymbolCollector::CreateMacroRef(const Node &node, const MacroInvocation &invocation)
{
    if (!invocation.target) {
        return;
    }
    if (invocation.decl && invocation.decl->astKind == ASTKind::MACRO_EXPAND_DECL) {
        CreateMacroRef(*invocation.decl, *invocation.decl->GetConstInvocation());
    }
    if (!node.curFile) {
        return;
    }
    auto begin = invocation.identifierPos;
    if (!invocation.fullNameDotPos.empty()) {
        begin = invocation.fullNameDotPos.back() + 1;
    }
    auto end = begin + CountUnicodeCharacters(invocation.identifier);
    SymbolLocation loc{begin, end, node.curFile->filePath};
    Ref refInfo{.location = loc, .kind = RefKind::REFERENCE, .container = GetContextID()};
    (void)symbolRefMap[GetDeclSymbolID(*invocation.target)].emplace_back(refInfo);
}

void SymbolCollector::CreateNamedArgRef(const CallExpr &ce)
{
    auto fd = ce.resolvedFunction;
    if (!fd || !fd->funcBody || fd->funcBody->paramLists.empty()) {
        return;
    }
    if (!ce.curFile) {
        return;
    }
    auto filePath = ce.curFile->filePath;
    std::unordered_map<std::string, SymbolID> namedParamSymbols;
    CJC_NULLPTR_CHECK(fd->funcBody->paramLists[0]);
    for (auto &param : fd->funcBody->paramLists[0]->params) {
        CJC_NULLPTR_CHECK(param);
        if (param->isNamedParam) {
            (void)namedParamSymbols.emplace(param->identifier, GetDeclSymbolID(*param));
        }
    }
    for (auto &arg : ce.args) {
        if (arg->name.Empty()) {
            continue;
        }
        auto found = namedParamSymbols.find(arg->name);
        if (found == namedParamSymbols.end()) {
            continue;
        }
        SymbolLocation loc{arg->name.Begin(), arg->name.Begin() + CountUnicodeCharacters(arg->name), filePath};
        Ref refInfo{.location = loc, .kind = RefKind::REFERENCE, .container = GetContextID()};
        (void)symbolRefMap[found->second].emplace_back(refInfo);
    }
}

void SymbolCollector::CollectRelations(
    const std::unordered_set<Ptr<InheritableDecl>> &inheritableDecls)
{
    for (auto &id : inheritableDecls) {
        for (auto &type : id->inheritedTypes) {
            auto decl = Ty::GetDeclPtrOfTy(type->ty);
            if (decl == nullptr || type->ty->IsObject()) { // Ignore core object.
                continue;
            }
            auto subject = GetDeclSymbolID(*decl);
            auto predicate = RelationKind::BASE_OF;
            auto object = GetDeclSymbolID(*id);
            if (auto ed = DynamicCast<const ExtendDecl *>(id.get())) {
                predicate = RelationKind::EXTEND;
                auto beingExtendDecl = Ty::GetDeclPtrOfTy(ed->extendedType->ty);
                if (beingExtendDecl == nullptr || ed->extendedType->ty->IsObject()) { // Ignore core object.
                    continue;
                }
                object = GetDeclSymbolID(*beingExtendDecl);
            }
            Relation relation{subject, predicate, object};
            (void)relations.emplace_back(relation);
        }

        for (auto &member : id->GetMemberDecls()) {
            bool ignore =
                member->astKind == ASTKind::VAR_DECL || member->TestAttr(Attribute::CONSTRUCTOR);
            if (ignore || !Ty::IsTyCorrect(member->ty)) {
                continue;
            }
            bool definedOverride = member->TestAttr(Attribute::REDEF, Attribute::STATIC) ||
                                   (member->TestAnyAttr(Attribute::PUBLIC, Attribute::PROTECTED) &&
                                    !member->TestAttr(Attribute::STATIC));
            if (definedOverride && id->astKind == ASTKind::CLASS_DECL) {
                if (auto parent =
                        FindOverriddenMemberFromSuperClass(*member, *StaticCast<ClassDecl *>(id))) {
                    Relation relation{.subject = GetDeclSymbolID(*parent),
                                      .predicate = RelationKind::RIDDEND_BY,
                                      .object = GetDeclSymbolID(*member)};
                    (void)relations.emplace_back(relation);
                    CollectNamedParam(parent, member.get());
                }
            }
            if (!member->TestAttr(Attribute::PUBLIC)) {
                continue;
            }
            auto parents = FindImplMemberFromInterface(*member, *id);
            for (auto parent : parents) {
                Relation relation{.subject = GetDeclSymbolID(*parent),
                                  .predicate = RelationKind::RIDDEND_BY,
                                  .object = GetDeclSymbolID(*member)};
                (void)relations.emplace_back(relation);
                CollectNamedParam(parent, member.get());
            }
        }
    }
}

Ptr<Decl> SymbolCollector::FindOverriddenMember(const Decl &member, const InheritableDecl &id,
                                               const TypeSubst &typeMapping,
                                               const std::function<bool(const Decl &)> &satisfy)
{
    Ptr<Decl> found = nullptr;
    for (auto &it : id.GetMemberDeclPtrs()) {
        CJC_NULLPTR_CHECK(it);
        if (!Ty::IsTyCorrect(it->ty) || !satisfy(*it)) {
            continue;
        }
        auto memberTy = tyMgr.GetInstantiatedTy(it->ty, typeMapping);
        if (member.astKind == ASTKind::PROP_DECL) {
            if (memberTy == member.ty) {
                found = it;
            }
            break;
        } else if (member.astKind != ASTKind::FUNC_DECL) {
            continue;
        }
        if (member.TestAttr(Attribute::GENERIC)) {
            TypeSubst mapping = tyMgr.GenerateGenericMappingFromGeneric(
                *static_cast<FuncDecl *>(it.get()), *static_cast<const FuncDecl *>(&member));
            memberTy = tyMgr.GetInstantiatedTy(memberTy, mapping);
        }
        if (tyMgr.IsFuncParameterTypesIdentical(*StaticCast<FuncTy *>(memberTy),
                                                *StaticCast<FuncTy *>(member.ty))) {
            found = it;
            break;
        }
    }
    return found;
}

Ptr<Decl> SymbolCollector::FindOverriddenMemberFromSuperClass(const Decl &member,
                                                             const ClassDecl &cd)
{
    auto condition = [&member](const Decl &parent) {
        return IsFitBaseMemberOverriddenCondition(parent, member) &&
               ((parent.TestAnyAttr(Attribute::OPEN, Attribute::ABSTRACT) &&
                 parent.TestAnyAttr(Attribute::PUBLIC, Attribute::PROTECTED)) ||
                (parent.TestAttr(Attribute::STATIC) && !parent.TestAttr(Attribute::PRIVATE)));
    };
    TypeSubst typeMapping;
    Ptr<const ClassDecl> current = &cd;
    while (current != nullptr) {
        Ptr<ClassDecl> sd = nullptr;
        for (auto &it : current->inheritedTypes) {
            if (!Ty::IsTyCorrect(it->ty) || !it->ty->IsClass()) {
                continue;
            }
            sd = StaticCast<ClassDecl *>(Ty::GetDeclPtrOfTy(it->ty));
            CJC_NULLPTR_CHECK(sd); // When ty is class and correct, sd must be non-null.
            typeMapping.merge(GenerateTypeMapping(sd->GetGeneric(), it->ty->typeArgs));
            if (auto parent = FindOverriddenMember(member, *sd, typeMapping, condition)) {
                return parent;
            }
            break;
        }
        current = sd;
    }
    return nullptr;
}

std::vector<Ptr<Decl>> SymbolCollector::FindImplMemberFromInterface(const Decl &member, const InheritableDecl &id)
{
    auto condition = [&member](const Decl &parent) {
        return IsFitBaseMemberOverriddenCondition(parent, member);
    };
    std::vector<Ptr<Decl>> ret;
    std::unordered_set<Ptr<Decl>> searched;
    std::queue<std::pair<Ptr<const InheritableDecl>, TypeSubst>> workList;
    workList.push(std::make_pair(&id, TypeSubst{}));
    while (!workList.empty()) {
        auto [curDecl, mapping] = workList.front();
        workList.pop();
        for (auto &it : curDecl->inheritedTypes) {
            if (!Ty::IsTyCorrect(it->ty)) {
                continue;
            }
            auto interfaceDecl = DynamicCast<InterfaceDecl *>(Ty::GetDeclPtrOfTy(it->ty));
            if (interfaceDecl == nullptr) {
                continue;
            }
            if (auto [_, success] = searched.emplace(interfaceDecl); !success) {
                continue;
            }
            auto currentMapping =
                GenerateTypeMapping(interfaceDecl->GetGeneric(), it->ty->typeArgs);
            currentMapping.insert(mapping.begin(), mapping.end());
            if (auto found =
                    FindOverriddenMember(member, *interfaceDecl, currentMapping, condition)) {
                (void)ret.emplace_back(found);
            } else {
                // If not found in current super interface, found from parent if exists.
                workList.push(std::make_pair(interfaceDecl, currentMapping));
            }
        }
    }
    return ret;
}

void SymbolCollector::CollectNamedParam(Ptr<const Decl> parent, Ptr<const Decl> member)
{
    std::unordered_map<std::string, std::pair<SymbolID, SymbolID>> record;
    auto pfd = DynamicCast<FuncDecl *>(parent);
    if (!pfd) {
        return;
    }
    for (const auto &pl : pfd->funcBody->paramLists) {
        for (const auto &fp : pl->params) {
            if (!fp->isNamedParam) {
                continue;
            }
            auto parentId = GetDeclSymbolID(*fp);
            (void)record.emplace(fp->identifier, std::make_pair(parentId, INVALID_SYMBOL_ID));
        }
    }

    auto mfd = DynamicCast<FuncDecl *>(member);
    if (!mfd) {
        return;
    }
    for (const auto &pl : mfd->funcBody->paramLists) {
        for (const auto &fp : pl->params) {
            if (!fp->isNamedParam) {
                continue;
            }
            auto memberId = GetDeclSymbolID(*fp);
            if (record.find(fp->identifier) != record.end()) {
                SymbolID tempId = record[fp->identifier].first;
                record[fp->identifier] = std::make_pair(tempId, memberId);
            }
        }
    }

    for (const auto &iter : record) {
        if (iter.second.first != INVALID_SYMBOL_ID && iter.second.second != INVALID_SYMBOL_ID) {
            Relation re{iter.second.first, RelationKind::RIDDEND_BY, iter.second.second};
            (void)relations.emplace_back(re);
        }
    }
}

void SymbolCollector::UpdatePos(SymbolLocation &location, const Node &node,
                                const std::string &filePath)
{
    Range range = {location.begin, location.end};
    ark::ArkAST *arkAst = this->astMap[filePath].get();
    if (!arkAst) {
        return;
    }
    ark::UpdateRange(arkAst->tokens, range, node);
    location.begin = range.start;
    location.end = range.end;
}
} // namespace ark::lsp
