// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "NormalCompleterByParse.h"
#include "KeywordCompleter.h"

using namespace Cangjie;
using namespace Cangjie::AST;
using namespace Cangjie::FileUtil;
using namespace Cangjie::Utils;

namespace {
auto CollectAliasMap(const File &file)
{
    std::unordered_map<std::string, std::string> fileAliasMap;
    for (const auto &import : file.imports) {
        if (!import->IsReExport() || import->IsImportSingle() || import->IsImportAll()) {
            continue;
        }

        if (import->IsImportAlias() && import->IsReExport()) {
            (void)fileAliasMap.insert_or_assign(import->content.identifier, import->content.aliasName);
        }

        // ImportMulti maybe include ImportAlias
        for (const auto &item : import->content.items) {
            if (item.kind == ImportKind::IMPORT_ALIAS) {
                (void)fileAliasMap.insert_or_assign(item.identifier, item.aliasName);
            }
        }
    }
    return fileAliasMap;
}
}

namespace ark {
bool NormalCompleterByParse::Complete(const ArkAST &input, const Position pos)
{
    CompletionEnv env;
    if (!input.file || !input.file->curPackage) {
        return true;
    }
    env.prefix = prefix;
    env.curPkgName = input.file->curPackage->fullPackageName;
    env.parserAst = &input;
    env.cache = input.semaCache;

    int curTokenIndex = input.GetCurTokenByPos(pos, 0, static_cast<int>(input.tokens.size()) - 1);
    if (curTokenIndex > 0) {
        auto preToken = input.tokens[static_cast<int>(curTokenIndex - 1)];
        if (preToken == "@") {
            env.isAfterAT = true;
        }
    }

    // Complete all imported and accessible top-level decl.
    pkgAliasMap.merge(::CollectAliasMap(*input.file));
    for (auto const &declSet : importManager->GetImportedDecls(*input.file)) {
        // The first element is real imported name
        for (const auto &decl : declSet.second) {
            result.importDeclsSymID.insert(CompletionEnv::GetDeclSymbolID(*decl));
            if (decl->identifier == declSet.first) {
                env.CompleteNode(decl.get());
            } else {
                env.CompleteAliasItem(decl.get(), declSet.first);
            }
        }
    }

    // Complete all top-decls in files of current package, which not includes import spec.
    Ptr<Decl> topLevelDecl = CompleteCurrentPackages(input, pos, env);
    // No code completion needed in these declaration position
    bool isMinDecl = false;
    bool isInPrimaryCtor = false;
    std::unordered_set<ASTKind> trustList = {ASTKind::CLASS_DECL, ASTKind::ENUM_DECL, ASTKind::VAR_DECL,
        ASTKind::STRUCT_DECL, ASTKind::INTERFACE_DECL, ASTKind::FUNC_DECL, ASTKind::MACRO_DECL, ASTKind::FUNC_PARAM,
        ASTKind::TYPE_ALIAS_DECL};

    Walker(
        topLevelDecl,
        [&](auto node) {
            auto decl = DynamicCast<Decl>(node);
            if (node->astKind == ASTKind::PRIMARY_CTOR_DECL) {
                isInPrimaryCtor = true;
            }
            if (decl && decl->identifier.Begin() <= pos && pos <= decl->identifier.End()) {
                bool isParamFuncInPrimary = node->astKind == ASTKind::FUNC_PARAM && isInPrimaryCtor;
                if (trustList.find(node->astKind) != trustList.end() && !isParamFuncInPrimary) {
                    isMinDecl = true;
                }
            }
            return VisitAction::WALK_CHILDREN;
        },
        [&](auto node) {
            if (node->astKind == ASTKind::PRIMARY_CTOR_DECL) {
                isInPrimaryCtor = false;
            }
            return VisitAction::WALK_CHILDREN;
        })
        .Walk();

    if (isMinDecl) {
        return false;
    }

    // Complete own scope decl.
    if (topLevelDecl) {
        env.DeepComplete(topLevelDecl, pos);
    }

    AddImportPkgDecl(input, env);

    env.OutputResult(result);
    return true;
}

void NormalCompleterByParse::AddImportPkgDecl(const ArkAST &input, CompletionEnv &env)
{
    std::vector<Ptr<PackageDecl>> curImportedPackages = {};
    bool isValid = input.semaCache && input.semaCache->packageInstance &&
                   input.semaCache->packageInstance->ctx &&
                   input.semaCache->packageInstance->ctx->curPackage;
    if (isValid) {
        curImportedPackages = importManager->GetCurImportedPackages(env.curPkgName);
    }
    // Complete imported packageName for fully qualified name reference.
    // Include cjo import and source import.
    auto curModule = SplitFullPackage(env.curPkgName).first;
    for (auto &im : input.file->imports) {
        if (im->IsImportSingle()) {
            auto fullPackageName = im->content.ToString();
            if (!CompilerCangjieProject::GetInstance()->Denoising(fullPackageName).empty() ||
                CompilerCangjieProject::GetInstance()->IsCurModuleCjoDep(curModule, fullPackageName)) {
                env.AccessibleByString(im->content.identifier, "packageName");
            }
        } else if (im->IsImportAlias()) {
            std::string candidate;
            for (auto &prefix : im->content.prefixPaths) {
                candidate += prefix + CONSTANTS::DOT;
            }
            candidate += im->content.identifier;
            if (!CompilerCangjieProject::GetInstance()->Denoising(candidate).empty() ||
                CompilerCangjieProject::GetInstance()->IsCurModuleCjoDep(curModule, candidate)) {
                env.AccessibleByString(im->content.aliasName, "packageName");
            }
        }
    }
    for (const auto &it : curImportedPackages) {
        if (!it || it->identifier.Empty()) {
            continue;
        }
        if (isValid) {
            FillingDeclsInPackage(it->identifier, env, it);
        }
    }
}

Ptr<Decl> NormalCompleterByParse::CompleteCurrentPackages(const ArkAST &input, const Position pos,
                                                          CompletionEnv &env)
{
    // parse info fullPackageName is "", so we need a flag to complete init Func
    env.isInPackage = true;
    Ptr<Decl> innerDecl = nullptr;
    bool invalid = !input.file || !input.file->curPackage;
    if (invalid) {
        return innerDecl;
    }
    for (auto &file : input.file->curPackage->files) {
        if (!file) {
            continue;
        }
        for (auto &decl : file->decls) {
            if (!DealDeclInCurrentPackage(decl.get(), env)) {
                continue;
            }
            if (file->fileHash == input.file->fileHash && decl->GetBegin() <= pos &&
                pos <= decl->GetEnd()) {
                innerDecl = decl.get();
            }
        }
    }
    bool flag = !input.semaCache || !input.semaCache->file || !input.semaCache->file->curPackage ||
                input.semaCache->file->curPackage->files.empty();
    if (flag) {
        env.isInPackage = false;
        return innerDecl;
    }
    // add Sema topLevel completeItem
    for (auto &file : input.semaCache->file->curPackage->files) {
        if (!file || !file.get() || file->decls.empty()) {
            continue;
        }
        for (auto &decl : file->decls) {
            if (decl && decl->astKind != Cangjie::AST::ASTKind::EXTEND_DECL) {
                env.CompleteNode(decl.get(), false, false, true);
            }
        }
    }
    env.isInPackage = false;
    return innerDecl;
}

bool NormalCompleterByParse::DealDeclInCurrentPackage(Ptr<Decl> decl, CompletionEnv &env)
{
    if (!decl) {
        return false;
    }
    if (decl->astKind != Cangjie::AST::ASTKind::EXTEND_DECL) {
        env.CompleteNode(decl);
    }
    auto declName = decl->identifier;
    if (decl->astKind == Cangjie::AST::ASTKind::EXTEND_DECL) {
        auto *extendDecl = dynamic_cast<ExtendDecl *>(decl.get());
        if (!extendDecl) {
            return false;
        }
        declName = extendDecl->extendedType->ToString();
        auto posOfLeft = declName.Val().find_first_of('<');
        if (posOfLeft != std::string::npos) {
            declName = declName.Val().substr(0, posOfLeft);
        }
    }
    // update idMap && importedExternalDeclMap
    if (env.idMap.find(declName) != env.idMap.end()) {
        env.idMap[declName].push_back(decl);
        env.importedExternalDeclMap[declName] = std::make_pair(decl, false);
    } else {
        std::vector<Ptr<Decl>> temp = {decl};
        env.idMap[declName] = temp;
        env.importedExternalDeclMap[declName] = std::make_pair(decl, false);
    }
    return true;
}

void NormalCompleterByParse::FillingDeclsInPackage(const std::string &packageName,
                                                   CompletionEnv &env,
                                                   Ptr<const PackageDecl> pkgDecl)
{
    // deal external import pkgDecl
    if (pkgDecl == nullptr) {
        pkgDecl = importManager->GetPackageDecl(packageName);
        if (!pkgDecl) {
            env.OutputResult(result);
            return;
        }
    }
    if (pkgDecl == nullptr || pkgDecl->srcPackage == nullptr ||
        pkgDecl->srcPackage->files.empty()) {
        return;
    }
}

void NormalCompleterByParse::CompleteModuleName(const std::string &curModule)
{
    CompletionEnv env;
    for (const auto &item : Cangjie::LSPCompilerInstance::cjoLibraryMap) {
        env.AccessibleByString(item.first, "moduleName");
    }
    for (const auto &item : CompilerCangjieProject::GetInstance()->GetOneModuleDeps(curModule)) {
        if (!IsFullPackageName(item)) {
            continue;
        }
        env.AccessibleByString(SplitFullPackage(item).first, "moduleName");
    }
    env.OutputResult(result);
}

void NormalCompleterByParse::CompletePackageSpec(const ArkAST &input)
{
    if (!input.file || input.file->filePath.empty()) {
        return;
    }
    CompletionEnv env;
    std::string path = Normalize(input.file->filePath);
    std::string fullPkgName = CompilerCangjieProject::GetInstance()->GetFullPkgName(path);
    if (IsFromCIMapNotInSrc(fullPkgName)) {
        auto workspace = CompilerCangjieProject::GetInstance()->GetWorkSpace();
        std::string relativeDirName = fullPkgName;
        size_t found = fullPkgName.find(workspace);
        if (found != std::string::npos) {
            relativeDirName = relativeDirName.substr(found + workspace.length());
            // Remove leading separator if present
            if (!relativeDirName.empty() && (relativeDirName[0] == '/' || relativeDirName[0] == '\\')) {
                relativeDirName = relativeDirName.substr(1);
            }
        }
#ifdef _WIN32
        std::replace(relativeDirName.begin(), relativeDirName.end(), '\\', '.');
#else
        std::replace(relativeDirName.begin(), relativeDirName.end(), '/', '.');
#endif
        fullPkgName = relativeDirName;
    }
    env.AccessibleByString(fullPkgName, "packageName");
    env.OutputResult(result);
}

} // namespace ark
