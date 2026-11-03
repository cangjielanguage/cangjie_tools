// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "InternalImportDiag.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include "cangjie/AST/Walker.h"
#include "cangjie/Utils/CastingTemplate.h"
#include "cangjie/Utils/Utils.h"

namespace ark {
namespace {
using namespace Cangjie;
using namespace Cangjie::AST;
using UsedDecls = std::unordered_set<std::pair<std::string, Ptr<Decl>>, HashPair>;
using UsedPackages = std::unordered_map<std::string, UsedDecls>;

void AddExtendTargets(const ExtendDecl &extend, UsedPackages &used)
{
    auto addType = [&used](const Ptr<Type> type) {
        auto target = type ? type->GetTarget() : nullptr;
        if (target) {
            used[target->GetFullPackageName()].emplace(target->identifier.Val(), target);
        }
    };
    for (const auto &type : extend.inheritedTypes) {
        addType(type);
    }
    addType(extend.extendedType);
    if (extend.generic) {
        for (const auto &constraint : extend.generic->genericConstraints) {
            for (const auto &type : constraint->upperBounds) {
                addType(type);
            }
        }
    }
}

UsedPackages CollectUsedPackages(const Package &package)
{
    UsedPackages used;
    ConstWalker(&package, [](Ptr<const Node> node) {
        return node->astKind == ASTKind::IMPORT_SPEC ? VisitAction::SKIP_CHILDREN : VisitAction::WALK_CHILDREN;
    }, [&used](Ptr<const Node> node) {
        std::string identifier;
        if (auto ref = DynamicCast<const RefExpr *>(node)) {
            identifier = ref->ref.identifier.Val();
        } else if (auto type = DynamicCast<const RefType *>(node)) {
            identifier = type->ref.identifier.Val();
        }
        if (!node->GetTarget()) {
            return VisitAction::WALK_CHILDREN;
        }
        auto addTarget = [&used, &identifier](Ptr<Decl> target) {
            if (!target) {
                return;
            }
            auto &decls = used[target->fullPackageName];
            decls.emplace(identifier.empty() ? target->identifier.Val() : identifier, target);
            if (target->outerDecl) {
                decls.emplace(target->outerDecl->identifier.Val(), target->outerDecl);
                if (auto extend = DynamicCast<ExtendDecl>(target->outerDecl)) {
                    AddExtendTargets(*extend, used);
                }
            }
        };
        addTarget(node->GetTarget());
        for (auto target : node->GetTargets()) {
            addTarget(target);
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    return used;
}

bool HasUsage(const UsedPackages &used, const std::string &pkgName, const std::string &name, Ptr<Decl> decl)
{
    const auto found = used.find(pkgName);
    return found != used.end() && found->second.count({name, decl}) != 0;
}

bool IsWildcardImportUsed(const std::string &pkgName, const std::map<std::string, OrderedDeclSet> &members,
    const UsedPackages &used, UsedPackages &cached)
{
    const auto found = used.find(pkgName);
    if ((found != used.end() && !found->second.empty()) || !cached[pkgName].empty()) {
        return true;
    }
    for (const auto &member : members) {
        for (auto decl : member.second) {
            if (HasUsage(used, decl->fullPackageName, decl->identifier.Val(), decl)) {
                cached[pkgName].emplace(decl->identifier.Val(), decl);
                return true;
            }
        }
    }
    return false;
}

bool IsDeclImportUsed(const ImportSpec &import, const std::string &pkgName,
    const std::map<std::string, OrderedDeclSet> &members, const UsedPackages &used, UsedPackages &cached)
{
    const auto found = members.find(import.content.identifier);
    if (found == members.end()) {
        return false;
    }
    const auto name = import.IsImportAlias() ? import.content.aliasName.Val() : import.content.identifier.Val();
    for (auto decl : found->second) {
        if (cached[pkgName].count({name, decl}) != 0 || HasUsage(used, decl->fullPackageName, name, decl)) {
            cached[pkgName].emplace(name, decl);
            return true;
        }
    }
    return false;
}

bool IsMacroImportUsed(const ImportSpec &import, const std::string &pkgName,
    const std::map<std::string, OrderedDeclSet> &members, ImportManager &imports)
{
    const auto &macros = imports.GetUsedMacroDecls(*import.curFile);
    auto usedMacro = [&macros](const std::string &pkg, Ptr<Decl> decl) {
        const auto found = macros.find(pkg);
        return found != macros.end() && found->second.count(decl) != 0;
    };
    if (import.IsImportAll() || !import.content.isDecl) {
        const auto found = macros.find(pkgName);
        if (found != macros.end() && !found->second.empty()) {
            return true;
        }
    }
    for (const auto &member : members) {
        if (!import.IsImportAll() && import.content.isDecl && member.first != import.content.identifier.Val()) {
            continue;
        }
        for (auto decl : member.second) {
            if (usedMacro(import.content.isDecl ? pkgName : decl->fullPackageName, decl)) {
                return true;
            }
        }
    }
    return false;
}

bool IsUsed(const ImportSpec &import, ImportManager &imports, const UsedPackages &used, UsedPackages &cached)
{
    const auto manager = imports.GetCjoManager();
    const auto pkgName = manager->GetPackageNameByImport(import);
    const auto package = manager->GetPackage(pkgName);
    // Leave unresolved imports to their existing compiler diagnostics.
    if (pkgName.empty() || !package) {
        return true;
    }
    const auto &members = manager->GetPackageMembers(pkgName);
    bool isUsed = false;
    if (import.IsImportAll()) {
        isUsed = IsWildcardImportUsed(pkgName, members, used, cached);
    } else if (import.content.isDecl) {
        isUsed = IsDeclImportUsed(import, pkgName, members, used, cached);
    } else {
        const auto name = import.IsImportAlias() ? import.content.aliasName.Val() : import.content.identifier.Val();
        isUsed = HasUsage(used, pkgName, name, imports.GetPackageDecl(pkgName));
    }
    if (isUsed || !package->isMacroPackage || !import.curFile) {
        return isUsed;
    }
    return IsMacroImportUsed(import, pkgName, members, imports);
}

bool NeedsRefresh(const ImportSpec &import)
{
    return import.modifier && import.modifier->modifier == TokenKind::INTERNAL && !import.IsImportMulti() &&
        (import.IsImportAll() || import.content.isDecl) &&
        !import.TestAttr(Attribute::IMPLICIT_ADD) && !import.begin.IsZero() && !import.end.IsZero() &&
        !(import.TestAttr(Attribute::IMPORTED) && import.TestAttr(Attribute::FROM_COMMON_PART));
}

void RemoveImportDiagnostics(const std::string &filePath, const ImportSpec &import,
    const std::vector<DiagnosticToken> &diagnostics, Callbacks &callback)
{
    for (const auto &old : diagnostics) {
        if (old.diagFix && old.diagFix->removeImport &&
            old.range.start.line == import.begin.line - 1 &&
            old.range.start.column == import.begin.column - 1 &&
            old.range.end.line == import.end.line - 1 &&
            old.range.end.column == import.end.column - 1) {
            callback.RemoveDiagnostic(filePath, old);
        }
    }
}
} // namespace

void InternalImportDiag::Refresh(const Cangjie::AST::Package &package, Cangjie::ImportManager &imports,
    Cangjie::DiagnosticEngine &diag, Callbacks &callback, bool noSubPkg)
{
    const bool hasInternalImport = std::any_of(package.files.begin(), package.files.end(), [](const auto &file) {
        return std::any_of(file->imports.begin(), file->imports.end(), [](const auto &import) {
            return NeedsRefresh(*import);
        });
    });
    if (!hasInternalImport) {
        return;
    }
    const auto used = noSubPkg ? CollectUsedPackages(package) : UsedPackages{};
    UsedPackages cached;
    for (const auto &file : package.files) {
        const auto diagnostics = callback.GetDiagsOfCurFile(file->filePath);
        for (const auto &import : file->imports) {
            if (!NeedsRefresh(*import)) {
                continue;
            }
            RemoveImportDiagnostics(file->filePath, *import, diagnostics, callback);
            if (noSubPkg && !IsUsed(*import, imports, used, cached)) {
                diag.DiagnoseRefactor(DiagKindRefactor::sema_unused_import, MakeRange(import->begin, import->end),
                    import->content.ToString());
            }
        }
    }
}
} // namespace ark
