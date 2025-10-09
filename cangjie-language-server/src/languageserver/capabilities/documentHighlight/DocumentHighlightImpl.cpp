// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "DocumentHighlightImpl.h"
#include "../../CompilerCangjieProject.h"
using namespace std;
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Cangjie::Meta;

namespace ark {
void GetDocumentHighlightItems(unsigned int fileID, const Decl &decl, std::set<DocumentHighlight> &result,
                               const std::vector<Cangjie::Token> &tokens)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "GetDocumentHighlightItems in.");

    // push the decl
    if (fileID == decl.GetIdentifierPos().fileID) {
        int length = static_cast<int>(CountUnicodeCharacters(decl.identifier));
        // Processes the set and get functions of the prop.
        if (!decl.identifierForLsp.empty()) {
            length = static_cast<int>(CountUnicodeCharacters(decl.identifierForLsp));
        }
        Range range = GetDeclRange(decl, length);
        /* 1.primary constructor and the init function that is added by the compiler has COMPILER_ADD attribute,
        it can't be filtered
        case 1: primary constructor
        class CP{
          CP(){} // primary constructor
        }
        var cp = CP() // primary constructor

        case 2:  init function that is added by the compiler
        class CP(){}
        var cp = CP() // CP() is init function that is added by the compiler

        2.other decl provided by compiler should be filtered
        */
        if (decl.TestAttr(Cangjie::AST::Attribute::PRIMARY_CONSTRUCTOR)
            || (!decl.TestAttr(Cangjie::AST::Attribute::COMPILER_ADD) ||
                decl.TestAttr(Cangjie::AST::Attribute::IS_CLONED_SOURCE_CODE))) {
            UpdateRange(tokens, range, decl);
            (void) result.insert({TransformFromChar2IDE(range), DocumentHighlightKind::TEXT});
        }
    }
    std::string path = CompilerCangjieProject::GetInstance()->GetFilePathByID(DocumentHighlightImpl::curFilePath,
                                                                              fileID);
    auto pkgName = CompilerCangjieProject::GetInstance()->GetFullPkgName(path);
    auto package = CompilerCangjieProject::GetInstance()->GetSourcePackagesByPkg(pkgName);
    if (!package) { return; }
    auto users = FindDeclUsage(decl, *package);
    // push the users
    for (auto &user: users) {
        if (auto refExpr = DynamicCast<RefExpr*>(user); refExpr && (refExpr->isSuper || refExpr->isThis)) {
            continue;
        }
        if (fileID != user->GetBegin().fileID || (user->TestAttr(Cangjie::AST::Attribute::COMPILER_ADD) &&
                                                  !user->TestAttr(Cangjie::AST::Attribute::IS_CLONED_SOURCE_CODE)) ||
            IsZeroPosition(user)) {
            continue;
        }
        Range range = GetRangeFromNode(user, tokens);
        (void)result.insert({TransformFromChar2IDE(range), DocumentHighlightKind::TEXT});
    }
}

void HandlePropDecl(unsigned int fileID, const Decl &decl, std::set<DocumentHighlight> &result,
                    const std::vector<Cangjie::Token> &tokens)
{
    auto *pPropDecl = dynamic_cast<const PropDecl*>(&decl);
    if (pPropDecl && pPropDecl->outerDecl && pPropDecl->outerDecl->astKind == Cangjie::AST::ASTKind::EXTEND_DECL) {
        GetDocumentHighlightItems(fileID, decl, result, tokens);
    }
    auto funcDecls = GetInheritDecls(pPropDecl);
    for (auto item: funcDecls) {
        GetDocumentHighlightItems(fileID, *item, result, tokens);
    }
}

void HandleFuncAndPropDecl(unsigned int fileID, const Decl &decl, std::set<DocumentHighlight> &result,
    const std::vector<Cangjie::Token> &tokens)
{
    if (decl.astKind == Cangjie::AST::ASTKind::PROP_DECL) {
        HandlePropDecl(fileID, decl, result, tokens);
        return;
    }
    auto funcDecl = dynamic_cast<const FuncDecl*>(&decl);
    if (!funcDecl || !funcDecl->funcBody) { return; }
    // init function that is added by the compiler and primary constructor has CONSTRUCTOR_H attribute
    bool valid = (!funcDecl->funcBody->parentClassLike && !funcDecl->funcBody->parentStruct &&
                  !funcDecl->funcBody->parentEnum) ||
                 funcDecl->TestAttr(Cangjie::AST::Attribute::CONSTRUCTOR) ||
                 funcDecl->TestAttr(Cangjie::AST::Attribute::ENUM_CONSTRUCTOR);
    if (valid) {
        GetDocumentHighlightItems(fileID, decl, result, tokens);
    } else {
        // for extended function for class
        if (funcDecl->outerDecl && funcDecl->outerDecl->astKind == Cangjie::AST::ASTKind::EXTEND_DECL) {
            GetDocumentHighlightItems(fileID, decl, result, tokens);
        }
        if (!result.empty()) { return; }
        auto funcDecls = GetInheritDecls(funcDecl);
        for (auto item : funcDecls) {
            GetDocumentHighlightItems(fileID, *item, result, tokens);
        }
    }
}

std::string DocumentHighlightImpl::curFilePath = "";

void DocumentHighlightImpl::FindDocumentHighlights(const ArkAST &ast, std::set<DocumentHighlight> &result,
                                                   Position pos)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "DocumentHighlightImpl::FindDocumentHighlights in.");
    // update pos fileID
    pos.fileID = ast.fileID;
    // adjust position from IDE to AST
    pos = PosFromIDE2Char(pos);
    PositionIDEToUTF8(ast.tokens, pos, *ast.file);
    if (ast.IsFilterTokenInHighlight(pos)) { return; }
    // get curFilePath
    curFilePath = ast.file ? ast.file->filePath : "";
    std::vector<Symbol *> syms;
    std::vector<Ptr<Cangjie::AST::Decl> > decls;
    Ptr<Decl> secend = ast.GetDeclByPosition(pos, syms, decls, {true, false});
    std::vector<Ptr<Cangjie::AST::Decl> > originDecls = decls;
    std::vector<Symbol *> originSyms = syms;
    DealMemberParam(curFilePath, syms, decls, secend);
    if (secend == nullptr) {
        return;
    }
    for (auto& decl: decls) {
        if (!decl || decl->astKind == Cangjie::AST::ASTKind::PACKAGE_DECL) { return; }
        if (decl->TestAttr(Attribute::DEFAULT, Attribute::COMPILER_ADD)) {
            auto query = "_ = (" + std::to_string(decl->identifier.Begin().fileID) + ", " +
                         std::to_string(decl->identifier.Begin().line) +
                         ", " + std::to_string(decl->identifier.Begin().column) + ")";
            auto curSyms = SearchContext(ast.packageInstance->ctx, query);
            for (const auto &sym: curSyms) {
                if (sym->name != decl->identifier.Val()) { continue; }
                if (auto realDecl = DynamicCast<Decl*>(sym->node)) {
                    decl = realDecl;
                }
            }
        }
        HandleFuncAndPropDecl(pos.fileID, *decl, result, ast.tokens);
        DealInCurPackage(ast, pos, syms, result, decl);
    }
}

void DocumentHighlightImpl::DealInCurPackage(const ArkAST &ast, const Position &pos, const vector<Symbol *> &syms,
                                             set <DocumentHighlight> &result, Ptr<Decl> &decl)
{
    string genericDeclName = decl->identifier;
    if (decl->astKind == ASTKind::GENERIC_PARAM_DECL) {
        auto temp = ast.FindRealGenericParamDeclForExtend(genericDeclName, syms);
        if (temp) {
            decl = temp;
        }
    }
    if (!Is<FuncDecl>(decl.get())) {
        GetDocumentHighlightItems(pos.fileID, *decl, result, ast.tokens);
    }
    bool isInvalid = !(decl->outerDecl && ValidExtendIncludeGenericParam(decl->outerDecl) &&
                       decl->outerDecl->generic.get() &&
                       !decl->outerDecl->generic->typeParameters.empty());
    if (isInvalid) {
        return;
    }
    auto inheritableDecl = dynamic_cast<InheritableDecl*>(decl->outerDecl.get());
    if (!inheritableDecl) {
        return;
    }
    string pkgName = GetPkgNameFromNode(ast.file);
    auto extendDecls = CompilerCangjieProject::GetInstance()->GetExtendDecls(inheritableDecl, pkgName);
    for (auto &extendDecl: extendDecls) {
        if (!extendDecl || !extendDecl->generic) {
            continue;
        }
        for (auto &genericDecl: extendDecl->generic->typeParameters)
            if (genericDecl && genericDecl->identifier == genericDeclName) {
                GetDocumentHighlightItems(pos.fileID, *genericDecl, result, ast.tokens);
            }
    }
}
} // namespace ark
