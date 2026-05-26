// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "FindOverrideMethodsImpl.h"
#include "FindOverrideMethodsUtils.h"
#include "cangjie/Lex/Token.h"
#include "cangjie/Utils/FileUtil.h"
#include "cangjie/AST/Symbol.h"
#include "cangjie/Modules/ModulesUtils.h"
#include "../../common/Utils.h"
#include "../../CompilerCangjieProject.h"
#include "../../common/ItemResolverUtil.h"

namespace ark {
std::string FindOverrideMethodsImpl::curFilePath;
std::string FindOverrideMethodsImpl::fullPkgName;
std::unordered_map<Ptr<InheritableDecl>, std::unordered_map<std::string, std::string>>
    FindOverrideMethodsImpl::genericReplaceMap;
const std::string TAB = "    ";
const std::string NEWLINE = "\n";
const std::string SPACE = " ";

void ApplyReplace(std::unique_ptr<TypeDetail>& detail, const std::unordered_map<std::string, std::string>& replace)
{
    for (auto& item: replace) {
        detail->SetIdentifier(item.first, item.second);
    }
}

bool IsOverrideable(const Ptr<ClassLikeDecl> owner, const Ptr<Decl> member)
{
    // make sure member is member of owner before call this function
    if (auto funcDecl = DynamicCast<FuncDecl>(member)) {
        if ((funcDecl->TestAttr(Attribute::OPEN) ||
            (funcDecl->TestAttr(Attribute::ABSTRACT) && owner->TestAttr(Attribute::ABSTRACT)) &&
            !funcDecl->TestAttr(Attribute::CONSTRUCTOR))) {
                return true;
            }
    }

    if (auto propDecl = DynamicCast<PropDecl>(member)) {
        if (propDecl->TestAttr(Attribute::OPEN) ||
            (propDecl->TestAttr(Attribute::ABSTRACT) && owner->TestAttr(Attribute::ABSTRACT))) {
                return true;
            }
    }

    return false;
}

void FindOverrideMethodsImpl::AddFuncItemsToResult(const Ptr<Decl>& decl, const Ptr<InheritableDecl>& owner,
                          OverrideMethodsItem& item, const std::vector<FuncDecl*>& overridableMethods,
                          const std::vector<Ptr<ClassLikeDecl>>& canSuperCall)
{
    for (auto method: overridableMethods) {
        OverrideMethodInfo info;
        std::string superCallText = GetSuperFuncCall(owner, method, canSuperCall);
        auto funcDetail = ResolveFuncDetail(method);
        if (auto inheritableDecl = DynamicCast<InheritableDecl>(owner)) {
            if (genericReplaceMap.find(inheritableDecl) != genericReplaceMap.end()) {
                const auto& replace = genericReplaceMap.at(inheritableDecl);
                for (auto& rule: replace) {
                    funcDetail.SetGenericType(rule.first, rule.second);
                }
            }
        }
        info.deprecated = method->HasAnno(AnnotationKind::DEPRECATED);
        FilterModifiers(decl, funcDetail.modifiers);
        info.insertText = funcDetail.ToString() + " {\n" + superCallText + "}";
        item.overrideMethodInfos.emplace_back(info);
    }
}

void FindOverrideMethodsImpl::AddPropItemsToResult(const Ptr<Decl> decl, const Ptr<InheritableDecl> owner,
                                                   OverrideMethodsItem& item,
                                                   const std::vector<PropDecl*>& overridableProps,
                                                   const std::vector<Ptr<ClassLikeDecl>>& canSuperCall)
{
    for (auto prop: overridableProps) {
        auto propDetail = ResolvePropDetail(prop);
        if (auto inheritableDecl = DynamicCast<InheritableDecl>(decl)) {
            if (genericReplaceMap.find(inheritableDecl) != genericReplaceMap.end()) {
                const auto& replace = genericReplaceMap.at(inheritableDecl);
                for (auto& rule: replace) {
                    propDetail.SetGenericType(rule.first, rule.second);
                }
            }
        }
        FilterModifiers(decl, propDetail.modifiers);
        auto identifier = propDetail.identifier;
        std::string superCall = TAB + TAB + "super." + identifier + NEWLINE;
        bool isCanSuperCall =
            std::any_of(canSuperCall.begin(), canSuperCall.end(), [&owner](const Ptr<ClassLikeDecl>& decl) {
                return decl->curFile == owner->curFile && decl->begin == owner->begin && decl->end == owner->end;
            });
        if (prop->TestAttr(Attribute::ABSTRACT) || (!prop->TestAttr(Attribute::STATIC) &&!isCanSuperCall)) {
            superCall = TAB + TAB + "throw Exception(\"Prop not implemented.\")\n";
        }

        if (prop->TestAttr(Attribute::STATIC) && !isCanSuperCall) {
            superCall = TAB + TAB + owner->identifier.Val() + "." + identifier + NEWLINE;
        }
        std::string getter = "get() {\n" + superCall + TAB + "}";
        std::string setter;
        if (std::find(propDetail.modifiers.begin(), propDetail.modifiers.end(), "mut") !=
                propDetail.modifiers.end()) {
            setter = "set(v) {\n" + TAB + "}";
        }
        OverrideMethodInfo info;
        info.isProp = true;
        info.deprecated = prop->HasAnno(AnnotationKind::DEPRECATED);
        info.insertText = propDetail.ToString() + " {\n" + TAB + getter;
        if (!setter.empty()) {
            info.insertText += NEWLINE + TAB + setter;
        }
        info.insertText += "\n}";
        item.overrideMethodInfos.emplace_back(info);
    }
}

template <class T>
void FilterMemberDecls(Ptr<T> decl, std::vector<FuncDecl*>& implementedMethods,
                       std::vector<PropDecl*>& implementedDecls)
{
    for (auto& memberDecl: decl->GetMemberDecls()) {
        if (FuncDecl* funcDecl = DynamicCast<FuncDecl*>(memberDecl.get())) {
            implementedMethods.emplace_back(funcDecl);
            continue;
        }

        if (PropDecl* propDecl = DynamicCast<PropDecl*>(memberDecl.get())) {
            implementedDecls.emplace_back(propDecl);
            continue;
        }
    }
}

void ExtractReplace(const Ptr<InheritableDecl>& decl, std::unordered_map<Ptr<InheritableDecl>,
                    std::unordered_map<std::string, std::string>>& genericReplaceMap)
{
    if (auto inheritableDecl = DynamicCast<InheritableDecl>(decl)) {
        const auto& inheritedTypes = inheritableDecl->inheritedTypes;
        std::unordered_map<std::string, std::string> replace{};
        if (genericReplaceMap.find(decl) != genericReplaceMap.end()) {
            replace = genericReplaceMap[decl];
        }
        for (const auto& inheritedType: inheritedTypes) {
            Ptr<ClassLikeDecl> inheritedDecl = nullptr;
            if (auto clsTy = DynamicCast<ClassTy*>(inheritedType->GetTy())) {
                inheritedDecl = clsTy->declPtr;
            } else if (auto ifTy = DynamicCast<InterfaceTy*>(inheritedType->GetTy())) {
                inheritedDecl = ifTy->declPtr;
            }
            if (inheritedDecl && inheritedType->GetTy()) {
                auto originalDetail = ResolveType(inheritedDecl->GetTy());
                auto newDetail = ResolveType(inheritedType->GetTy());
                ApplyReplace(newDetail, replace);
                std::unordered_map<std::string, std::string> myReplace;
                originalDetail->Diff(newDetail, myReplace);
                genericReplaceMap[inheritedDecl] = myReplace;
            }
        }
    }
}

void FindOverrideMethodsImpl::FindOverrideMethods(const ArkAST &ast, FindOverrideMethodResult &result,
                                                  Cangjie::Position pos, bool isExtend)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "FindOverrideMethodsImpl::FindOverrideMethods in.");
    // update pos fileID
    pos.fileID = ast.fileID;
    // adjust position from IDE to AST
    pos = PosFromIDE2Char(pos);
    PositionIDEToUTF8(ast.tokens, pos, *ast.file);
    // get curFilePath
    curFilePath = ast.file ? ast.file->filePath : "";
    LowFileName(curFilePath);
    // check current token is the kind required in function CheckTokenKind(TokenKind)
    std::vector<Symbol *> syms;
    std::vector<Ptr<Cangjie::AST::Decl> > decls;
    Ptr<Decl> decl = ast.GetDeclByPosition(pos, syms, decls, {true, false});
    if (!decl && !isExtend) { return; }

    if (isExtend && syms.empty()) {
        // in extended type is basic type case:
        //      try to obtain extend decl, pos begin of extended type identifier before macro expand
        std::string query = "_ = (" + std::to_string(pos.fileID) + ", "
                            + std::to_string(pos.line) + ", " + std::to_string(pos.column) + ")";

        syms = SearchContext(ast.packageInstance->ctx, query);
    }

    if (syms.empty() || !syms[0] || (!isExtend && !DynamicCast<InheritableDecl*>(decl))) {
        return;
    }

    // in extend case, identifier is a class decl or others,
    // discard the search result and get the ExtendDecl by found symbols
    if (isExtend) {
        if (syms.size() <= 1 || !syms[1]) {
            return;
        }
        if (auto extendDecl = DynamicCast<ExtendDecl*>(syms[1]->node)) {
            decl = extendDecl;
        } else {
            return;
        }
    }

    if (!decl) {
        return;
    }

    if (auto inheritableDecl = DynamicCast<InheritableDecl *>(decl)) {
        std::vector<FuncDecl *> implementedMethods;
        std::vector<PropDecl *> implementedDecls;
        genericReplaceMap.clear();
        GetImplementedMethodsAndProps(ast, inheritableDecl, implementedMethods, implementedDecls);
        std::vector<Ptr<ClassLikeDecl>> canSuperCall = GetCanSuperCallDecls(decl);
        OverridableFuncAndPropMap overrideableMethodsAndPropMap;
        GetOverridableMethodsAndPropsMap(
            inheritableDecl, overrideableMethodsAndPropMap, implementedMethods, implementedDecls);

        for (const auto &mapItem : overrideableMethodsAndPropMap) {
            const Ptr<InheritableDecl> owner = mapItem.first;
            OverrideMethodsItem item;
            item.package = GetPkgNameFromNode(mapItem.first);
            item.identifier = mapItem.first->identifier;
            item.kind = mapItem.first->astKind == ASTKind::INTERFACE_DECL ? "interface"
                        : mapItem.first->astKind == ASTKind::CLASS_DECL   ? "class"
                                                                          : "others";
            AddFuncItemsToResult(decl, owner, item, mapItem.second.first, canSuperCall);
            AddPropItemsToResult(decl, owner, item, mapItem.second.second, canSuperCall);
            result.overrideMethods.emplace_back(item);
        }
    }
}

void FindOverrideMethodsImpl::GetImplementedMethodsAndProps(const ArkAST &ast, Ptr<InheritableDecl> decl,
    std::vector<FuncDecl*>& implementedMethods, std::vector<PropDecl*>& implementedDecls)
{
    if (auto classDecl = DynamicCast<ClassLikeDecl*>(decl)) {
        FilterMemberDecls<ClassLikeDecl>(classDecl, implementedMethods, implementedDecls);
    }

    if (auto structDecl = DynamicCast<StructDecl*>(decl)) {
        FilterMemberDecls<StructDecl>(structDecl, implementedMethods, implementedDecls);
    }

    if (auto enumDecl = DynamicCast<EnumDecl*>(decl)) {
        FilterMemberDecls<EnumDecl>(enumDecl, implementedMethods, implementedDecls);
    }

    if (auto* extendDecl = DynamicCast<ExtendDecl*>(decl)) {
        FilterMemberDecls<ExtendDecl>(extendDecl, implementedMethods, implementedDecls);
        auto extendTyDecl =  ast.GetDelFromType(extendDecl->extendedType);
        if (auto originDecl = DynamicCast<InheritableDecl*>(extendTyDecl)) {
            auto superTys = originDecl->GetAllSuperDecls();
            for (auto superTy: superTys) {
                FilterMemberDecls<ClassLikeDecl>(superTy, implementedMethods, implementedDecls);
            }
        }
    }
}

void FindOverrideMethodsImpl::GetOverridableMethodsAndPropsMap(Ptr<InheritableDecl> decl,
                                                               OverridableFuncAndPropMap& overrideableMethodsAndPropMap,
                                                               const std::vector<FuncDecl*>& implementedMethods,
                                                               const std::vector<PropDecl*>& implementedProps)
{
    std::vector<FuncDecl*> cachedFuncResult = implementedMethods;
    std::vector<PropDecl*> cachedPropResult = implementedProps;
    // check identical func decl in different level
    auto checkRepeatFunc = [&cachedFuncResult](FuncDecl* func) -> bool {
        return std::any_of(cachedFuncResult.begin(), cachedFuncResult.end(),
                           [func](const FuncDecl* cmpDecl)-> bool {
            // this function can‘t cover all cases
            return IsFuncSignatureIdentical(*cmpDecl, *func);
        });
    };

    auto checkRepeatProp = [&cachedPropResult](PropDecl* prop) -> bool {
        return std::any_of(cachedPropResult.begin(), cachedPropResult.end(),
                           [prop](const PropDecl* cmpDecl)-> bool {
            return prop->identifier == cmpDecl->identifier;
        });
    };

    ExtractReplace(decl, genericReplaceMap);

    auto superTys = decl->GetAllSuperDecls();
    for (auto superTy: superTys) {
        ExtractReplace(superTy, genericReplaceMap);
        auto& memberDecls = superTy->GetMemberDecls();
        for (auto& memberDecl: memberDecls) {
            if (auto funcDecl = DynamicCast<FuncDecl*>(memberDecl.get())) {
                if (IsOverrideable(superTy, funcDecl) && !checkRepeatFunc(funcDecl)) {
                    overrideableMethodsAndPropMap[superTy].first.emplace_back(funcDecl);
                }
                cachedFuncResult.emplace_back(funcDecl);
            }

            if (auto propDecl = DynamicCast<PropDecl*>(memberDecl.get())) {
                if (IsOverrideable(superTy, propDecl) && !checkRepeatProp(propDecl)) {
                    overrideableMethodsAndPropMap[superTy].second.emplace_back(propDecl);
                }
                cachedPropResult.emplace_back(propDecl);
            }
        }
    }
}
}
