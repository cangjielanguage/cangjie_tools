// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "LSPCompilerInstance.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "CjoManager.h"
#include "CompilerCangjieProject.h"
#include "DependencyGraph.h"
#include "cangjie/Driver/TempFileManager.h"
#include "common/Utils.h"
#ifdef __linux__
#include <malloc.h>
#endif

using namespace Cangjie;
using namespace AST;
using namespace Cangjie::FileUtil;
using namespace Cangjie::Utils;

namespace  {
inline bool ShouldSkipDecl(const Decl &decl)
{
    auto md = DynamicCast<const MacroDecl *>(&decl);
    const Decl &beCheckedDecl = md && md->desugarDecl ? *md->desugarDecl : decl;
    return beCheckedDecl.TestAnyAttr(Attribute::HAS_BROKEN, Attribute::IS_BROKEN) || !Ty::IsTyCorrect(beCheckedDecl.ty);
}

std::tuple<std::string, std::string> GetFullPackageNames(const ImportSpec& import)
{
    // Multi-imports are desugared after parser which should not be used for get package name.
    CJC_ASSERT(import.content.kind != ImportKind::IMPORT_MULTI);
    if (import.content.prefixPaths.empty()) {
        return std::tuple(import.content.identifier, "");
    }
    std::string fullPackageName = JoinStrings(import.content.prefixPaths, ".");
    if (import.content.kind == ImportKind::IMPORT_ALL) {
        return std::tuple(fullPackageName, "");
    }
    [[maybe_unused]] std::string maybePackageName = fullPackageName + "." + import.content.identifier;
    return std::tuple(maybePackageName, fullPackageName);
}
}

LSPCompilerInstance::LSPCompilerInstance(ark::Callbacks *cb, CompilerInvocation &invocation,
                                         DiagnosticEngine &diag, std::string realPkgName,
                                         const std::unique_ptr<ark::ModuleManager> &moduleManger)
    : CompilerInstance(invocation, diag), callback(cb), pkgNameForPath(std::move(realPkgName)),
      moduleManger(moduleManger)
{
    (void)ExecuteCompilerApi("SetSourceCodeImportStatus", &ImportManager::SetSourceCodeImportStatus,
                             &importManager, false);
}

std::unordered_map<std::string, ark::EdgeType> LSPCompilerInstance::UpdateUpstreamPkgs()
{
    const auto packages = GetSourcePackages();
    if (packages.empty()) {
        return {};
    }

    std::string curModule;
    std::unordered_set<std::string> depends;
    if (!pkgNameForPath.empty()) {
        curModule = SplitQualifiedName(pkgNameForPath).front();
        depends = ark::CompilerCangjieProject::GetInstance()->GetOneModuleDeps(curModule);
    }

    std::set<std::string> depPkgs;
    std::unordered_map<std::string, ark::EdgeType> depPkgsEdges;
    std::unordered_map<TokenKind, ark::EdgeType> edgeKindMap = {
        {TokenKind::PUBLIC, ark::EdgeType::PUBLIC},
        {TokenKind::PROTECTED, ark::EdgeType::PROTECTED},
        {TokenKind::INTERNAL, ark::EdgeType::INTERNEL},
        {TokenKind::PRIVATE, ark::EdgeType::PRIVATE}
    };
    for (const auto &file : packages[0]->files) {
        for (auto &import : file->imports) {
            if (import->IsImportMulti()) {
                continue;
            }
            TokenKind modifier = import->modifier ? import->modifier->modifier : TokenKind::PRIVATE;
            // Get real dependent package.
            auto [package, orPackage] = ::GetFullPackageNames(*import);
            package = Denoising(package);
            orPackage = Denoising(orPackage);
            if (package.empty() && orPackage.empty()) {
                continue;
            }
            std::string realDep = (package.size() > orPackage.size()) ? package : orPackage;
            if (pkgNameForPath.empty() || realDep.empty()) {
                if (depPkgsEdges.find(realDep) == depPkgsEdges.end() || depPkgsEdges[realDep] < edgeKindMap[modifier]) {
                    depPkgsEdges[realDep] = edgeKindMap[modifier];
                }
                depPkgs.insert(realDep);
                continue;
            }
            auto realModule = SplitQualifiedName(realDep).front();
            if (curModule != realModule && depends.count(realModule) == 0) {
                continue;
            }
            if (depPkgsEdges.find(realDep) == depPkgsEdges.end() || depPkgsEdges[realDep] < edgeKindMap[modifier]) {
                depPkgsEdges[realDep] = edgeKindMap[modifier];
            }
            depPkgs.insert(realDep);
        }
    }
    upstreamPkgs = depPkgs;
    return depPkgsEdges;
}

void LSPCompilerInstance::UpdateDepGraph(bool isIncrement, const std::string &prePkgName)
{
    (void)UpdateUpstreamPkgs();
    {
        std::unique_lock<std::shared_mutex> lck(mtx);
        if (pkgNameForPath.empty() && !invocation.globalOptions.packagePaths.empty()) {
            pkgNameForPath = invocation.globalOptions.packagePaths[0];
            dependentPackageMap[pkgNameForPath].isInModule = false;
        }
        if (dependentPackageMap.count(pkgNameForPath)) {
            for (auto &item : dependentPackageMap[pkgNameForPath].importPackages) {
                if (!dependentPackageMap.count(item)) {
                    continue;
                }
                dependentPackageMap[item].downstreamPkgs.erase(pkgNameForPath);
            }
        }

        if (!prePkgName.empty() && prePkgName != pkgNameForPath) {
            dependentPackageMap[pkgNameForPath].downstreamPkgs =
                dependentPackageMap[prePkgName].downstreamPkgs;
            for (const auto &item : dependentPackageMap) {
                if (item.first == pkgNameForPath) {
                    continue;
                }
                if (dependentPackageMap[item.first].importPackages.count(pkgNameForPath)) {
                    dependentPackageMap[pkgNameForPath].downstreamPkgs.insert(item.first);
                }
            }
            if (dependentPackageMap.count(prePkgName)) {
                for (auto &iter : dependentPackageMap[prePkgName].importPackages) {
                    dependentPackageMap[iter].downstreamPkgs.erase(prePkgName);
                }
            }
            dependentPackageMap.erase(prePkgName);
        }

        dependentPackageMap[pkgNameForPath].importPackages = upstreamPkgs;
        dependentPackageMap[pkgNameForPath].inDegree = upstreamPkgs.size();
        if (isIncrement) {
            for (const auto &item : upstreamPkgs) {
                if (!dependentPackageMap.count(item)) {
                    dependentPackageMap[pkgNameForPath].importPackages.erase(item);
                    dependentPackageMap[pkgNameForPath].inDegree =
                        dependentPackageMap[pkgNameForPath].importPackages.size();
                    continue;
                }
                dependentPackageMap[item].downstreamPkgs.insert(pkgNameForPath);
            }
        }
    }
}

void LSPCompilerInstance::UpdateDepGraph(
    const std::unique_ptr<ark::DependencyGraph> &graph, const std::string &fullPkgName)
{
    auto edges = UpdateUpstreamPkgs();
    graph->UpdateDependencies(fullPkgName, upstreamPkgs, edges);
}

void LSPCompilerInstance::PreCompileProcess()
{
    diag.Reset();
    diag.SetSourceManager(&GetSourceManager());

    (void)Parse();
    // parse completion need condition compile
    (void)ConditionCompile();
    AddSourceToMember();
}

void LSPCompilerInstance::CompilePassForComplete(
    const std::unique_ptr<ark::CjoManager> &cjoManager,
    const std::unique_ptr<ark::DependencyGraph> &graph, Position pos, const std::string &name)
{
    // Faster Completion needs pass: Parse, ConditionCompile and ImportPackage.
    diag.Reset();
    diag.SetSourceManager(&GetSourceManager());
    (void)Parse();
    (void)ConditionCompile();
    const auto filePath = GetSourceManager().GetSource(pos.fileID).path;
    auto file = GetFileByPath(filePath).get();
    // If the position is not in ImportSpec, do not need to ImportPackage.
    if (file && !ark::InImportSpec(*file, pos) && name != "SignatureHelp") {
        return;
    }
    ImportCjoToManager(cjoManager, graph);
    (void)ImportPackage();
}

Ptr<File> LSPCompilerInstance::GetFileByPath(const std::string& filePath)
{
    const auto package = GetSourcePackages()[0];
    for (auto &file : package->files) {
        if (file->filePath == filePath) {
            return file;
        }
    }
    return nullptr;
}

std::unordered_set<std::string> LSPCompilerInstance::GetAllImportedCjo(
    const std::string &pkgName, std::unordered_map<std::string, bool> &isVisited)
{
    std::unordered_set<std::string> res;
    if (isVisited[pkgName]) {
        return res;
    }
    isVisited[pkgName] = true;
    if (!dependentPackageMap.count(pkgName)) {
        return res;
    }
    auto deps = dependentPackageMap[pkgName].importPackages;

    if (deps.empty()) {
        return res;
    }
    for (const auto &dependent : dependentPackageMap[pkgName].importPackages) {
        auto temp = GetAllImportedCjo(dependent, isVisited);
        res.insert(temp.begin(), temp.end());
    }
    res.insert(deps.begin(), deps.end());
    return res;
}

bool LSPCompilerInstance::ToImportPackage(const std::string &curModuleName,
                                          const std::string &cjoPackage)
{
    std::string cjoModuleName = ark::SplitFullPackage(cjoPackage).first;
    if (astDataMap.find(cjoPackage) == astDataMap.end() || astDataMap[cjoPackage].first.empty() ||
        cjoModuleName.empty()) {
        return false;
    }
    if (curModuleName.empty()) {
        return true;
    }
    auto found = moduleManger->requireAllPackages.find(curModuleName);
    if (found != moduleManger->requireAllPackages.end() && found->second.count(cjoModuleName)) {
        return true;
    }
    return false;
}

void LSPCompilerInstance::ImportUsrPackage(const std::string &curModuleName)
{
    std::unordered_map<std::string, bool> isVisited;
    const std::unordered_set<std::string> cjoPackageAll = GetAllImportedCjo(pkgNameForPath, isVisited);
    for (const auto &cjoPackage : cjoPackageAll) {
        if (ToImportPackage(curModuleName, cjoPackage)) {
            importManager.SetPackageCjoCache(cjoPackage, astDataMap[cjoPackage].first);
        }
    }
}

void LSPCompilerInstance::ImportUsrCjo(const std::string &curModuleName)
{
    if (usrCjoFileCacheMap.count(curModuleName) != 0) {
        CjoCacheMap &cjoCacheMap = usrCjoFileCacheMap[curModuleName];
        for (auto &item : cjoCacheMap) {
            importManager.SetPackageCjoCache(item.first, item.second);
        }
    }
}

void LSPCompilerInstance::ImportCjoToManager(
    const std::unique_ptr<ark::CjoManager> &cjoManager, const std::unique_ptr<ark::DependencyGraph> &graph)
{
    std::string curModuleName = invocation.globalOptions.moduleName;

    // Import stdlib cjo, priority is low.
    for (const auto &cjoCache : cjoFileCacheMap) {
        importManager.SetPackageCjoCache(cjoCache.first, cjoCache.second);
    }

    // import cjo for bin dependencies in cjpm.toml
    ImportUsrCjo(curModuleName);

    // Import user's source code, priority is high.
    const auto allDependencies = graph->FindAllDependencies(pkgNameForPath);
    for (auto &package : allDependencies) {
        auto cjoCache = cjoManager->GetData(package);
        if (!cjoCache) {
            continue;
        }
        importManager.SetPackageCjoCache(package, *cjoCache);
    }
}

void LSPCompilerInstance::IndexCjoToManager(
    const std::unique_ptr<ark::CjoManager> &cjoManager, const std::unique_ptr<ark::DependencyGraph> &graph)
{
    // Import stdlib's cjo, priority is low.
    for (const auto &cjoCache : cjoFileCacheMap) {
        importManager.SetPackageCjoCache(cjoCache.first, cjoCache.second);
    }
    for (const auto &item : usrCjoFileCacheMap) {
        for (const auto &cjoCache : item.second) {
            importManager.SetPackageCjoCache(cjoCache.first, cjoCache.second);
        }
    }
}

/**
 * @brief The rest of the compilation process is performed, and the astdata data in the cache is updated and the
 * downstream package is marked as stale.
 *
 * @param cjoManager Read cjo cache and update cjo cache and state
 * @param graph
 * @return true
 * @return false
 */
bool LSPCompilerInstance::CompileAfterParse(
    const std::unique_ptr<ark::CjoManager> &cjoManager, const std::unique_ptr<ark::DependencyGraph> &graph)
{
    ImportCjoToManager(cjoManager, graph);
    (void)ImportPackage();
    if (!ark::CompilerCangjieProject::GetInstance()->isIdentical) {
        return false;
    }
    macroExpandSuccess = MacroExpand();
    (void)Sema();
    (void)ExecuteCompilerApi("DeleteASTLoaders", &ImportManager::DeleteASTLoaders, this->importManager);
    const auto packages = GetSourcePackages();
    if (packages.empty()) {
        return false;
    }
    if (pkgNameForPath.empty()) {
        return false;
    }
    ark::CjoData cjoData;
    std::vector<uint8_t> data;
    MarkBrokenDecls(*packages[0]);
    (void)ExportAST(false, data, *packages[0]);
    auto oldData = cjoManager->GetData(pkgNameForPath);
    bool changed = cjoManager->CheckChanged(pkgNameForPath, data);
    cjoData.data = data;
    cjoData.status = ark::DataStatus::FRESH;
    cjoManager->SetData(pkgNameForPath, cjoData);
    return changed;
}

std::vector<std::string> LSPCompilerInstance::GetTopologySort()
{
    auto tempDependentPackageMap = dependentPackageMap;
    std::vector<std::string> result;
    std::queue<std::string> que;
    for (auto &iter : tempDependentPackageMap) {
        if (!iter.second.isInModule) {
            continue;
        }
        if (iter.second.inDegree == 0) {
            que.emplace(iter.first);
        }
    }
    while (!que.empty()) {
        std::string pkgName = que.front();
        result.push_back(pkgName);
        que.pop();
        if (!tempDependentPackageMap.count(pkgName)) {
            continue;
        }
        for (auto &outEdge : tempDependentPackageMap[pkgName].downstreamPkgs) {
            if (tempDependentPackageMap[outEdge].inDegree > 0) {
                tempDependentPackageMap[outEdge].inDegree--;
            }
            if (tempDependentPackageMap[outEdge].inDegree == 0) {
                que.emplace(outEdge);
            }
        }
    }
    return result;
}

void LSPCompilerInstance::SetCjoPathInModules(const std::string &cangjieHome,
                                              const std::string &cangjiePath)
{
    auto invocation = CompilerInvocation();
#ifdef _WIN32
    const std::string separator = ";";
#else
    const std::string separator = ":";
#endif
    if (cangjieHome.empty()) {
        return;
    }
    if (!ark::Options::GetInstance().IsOptionSet("test") && ark::MessageHeaderEndOfLine::GetIsDeveco()) {
        invocation.globalOptions.target.arch = Triple::ArchType::AARCH64;
        invocation.globalOptions.target.os = Triple::OSType::LINUX;
        invocation.globalOptions.target.env = Triple::Environment::OHOS;
    }
    const auto libPathName = invocation.globalOptions.GetCangjieLibTargetPathName();
    if (libPathName.empty()) {
        return;
    }
    auto stdLibraryPath = JoinPath(JoinPath(cangjieHome, "modules"), libPathName);
    cjoPathInModules.emplace_back(stdLibraryPath);

    if (!cangjiePath.empty()) {
        auto tmpVec = SplitString(cangjiePath, separator);
        std::copy(tmpVec.begin(), tmpVec.end(), std::back_inserter(cjoPathInModules));
    }
}

void LSPCompilerInstance::ReadCjoFileOneModule(const std::string &modulePath)
{
    std::string packageName;
    const std::string moduleName = GetDirName(modulePath);
    std::vector<std::string> packages = {};
    for (auto &file : GetAllFilesUnderCurrentPath(modulePath, "cjo")) {
        auto cjoPath = JoinPath(modulePath, file);
        if (!FileExist(cjoPath)) {
            continue;
        }
        std::vector<uint8_t> tmpAST;
        std::string failedReason;
        if (ReadBinaryFileToBuffer(cjoPath, tmpAST, failedReason)) {
            packageName = GetFileNameWithoutExtension(file);
            (void)cjoFileCacheMap.emplace(packageName, tmpAST);
            cjoPathSet.insert(cjoPath);
            packages.emplace_back(packageName);
        }
    }

    cjoLibraryMap[moduleName] = packages;
}

void LSPCompilerInstance::ReadCjoFileOneModuleExternal(const std::string &modulesPath)
{
    std::string packageName;
    for (auto &file : GetAllFilesUnderCurrentPath(modulesPath, "cjo")) {
        auto cjoPath = JoinPath(modulesPath, file);
        if (!FileExist(cjoPath)) {
            continue;
        }
        std::vector<uint8_t> tmpAST;
        std::string failedReason;
        if (ReadBinaryFileToBuffer(cjoPath, tmpAST, failedReason)) {
            packageName = GetFileNameWithoutExtension(file);
            (void)cjoFileCacheMap.emplace(packageName, tmpAST);
            cjoPathSet.insert(cjoPath);
            size_t pos = packageName.find('.');
            std::string moduleName = (pos != std::string::npos) ? packageName.substr(0, pos) : packageName;
            auto it = cjoLibraryMap.find(moduleName);
            if (it != cjoLibraryMap.end()) {
                it->second.push_back(packageName);
            } else {
                cjoLibraryMap[moduleName] = {packageName};
            }
        }
    }
}

void LSPCompilerInstance::InitCacheFileCacheMap()
{
    for (const auto &dirPath : cjoPathInModules) {
        for (auto &modulePath : GetAllDirsUnderCurrentPath(dirPath)) {
            ReadCjoFileOneModule(modulePath);
        }
        ReadCjoFileOneModuleExternal(dirPath);
    }
}

void LSPCompilerInstance::UpdateUsrCjoFileCacheMap(
    std::string &moduleName, std::unordered_map<std::string, std::string> &cjoRequiresMap)
{
    std::string packageName;
    std::string fullPkgName;
    std::string requireCjoPath;
    CjoCacheMap requiresMap = {};
    for (auto &require : cjoRequiresMap) {
        fullPkgName = require.first;
        requireCjoPath = require.second;
        if (!FileExist(requireCjoPath)) {
            continue;
        }
        std::vector<uint8_t> tmpAST;
        std::string failedReason;
        if (ReadBinaryFileToBuffer(requireCjoPath, tmpAST, failedReason)) {
            (void)requiresMap.emplace(fullPkgName, tmpAST);
            cjoPathSet.insert(requireCjoPath);
            auto curModuleName = ark::SplitFullPackage(fullPkgName).first;
            packageName = ark::SplitFullPackage(fullPkgName).second;
            cjoLibraryMap[curModuleName].emplace_back(packageName);
        }
    }
    usrCjoFileCacheMap.emplace(moduleName, requiresMap);
}

void LSPCompilerInstance::MarkBrokenDecls(Package &pkg)
{
    Walker(&pkg, [](Ptr<Node> node) {
        if (In(node->astKind, {ASTKind::PACKAGE, ASTKind::FILE})) {
            return VisitAction::WALK_CHILDREN;
        }
        const auto decl = DynamicCast<Decl *>(node);
        if (!decl) {
            return VisitAction::SKIP_CHILDREN;
        }
        if (ShouldSkipDecl(*decl)) {
            decl->doNotExport = true;
        } else if (decl->IsNominalDecl()) {
            for (auto member : decl->GetMemberDeclPtrs()) {
                if (ShouldSkipDecl(*member)) {
                    member->doNotExport = true;
                }
            }
        }
        return VisitAction::SKIP_CHILDREN;
    }).Walk();
}

std::string LSPCompilerInstance::Denoising(std::string candidate)
{
    return ark::CompilerCangjieProject::GetInstance()->Denoising(candidate);
}