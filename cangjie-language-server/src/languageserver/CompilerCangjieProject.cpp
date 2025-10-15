// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CompilerCangjieProject.h"
#include <memory>
#include <string>
#include "CjoManager.h"
#include "common/Utils.h"
#include "common/SyscapCheck.h"
#include "index/CjdIndex.h"
#include "index/IndexStorage.h"
#include "logger/Logger.h"
#ifdef __linux__
#include <malloc.h>
#endif
#include <cangjie/Modules/ModulesUtils.h>
#if __APPLE__
#include <malloc/malloc.h>
#endif
using namespace Cangjie;
using namespace Cangjie::AST;
using namespace CONSTANTS;
using namespace std::chrono;
using namespace Cangjie::FileUtil;
using namespace Cangjie::Utils;
using namespace Cangjie::Modules;

namespace {
enum class PkgRelation { NONE, CHILD, SAME_MODULE };
PkgRelation GetPkgRelation(const std::string &srcFullPackageName, const std::string &targetFullPackageName)
{
    if (srcFullPackageName.rfind(targetFullPackageName, 0) == 0 ||
        targetFullPackageName.rfind(srcFullPackageName, 0) == 0) {
        return PkgRelation::CHILD;
    }
    auto srcRoot = Utils::SplitQualifiedName(srcFullPackageName).front();
    auto targetRoot = Utils::SplitQualifiedName(targetFullPackageName).front();
    return srcRoot == targetRoot ? PkgRelation::SAME_MODULE : PkgRelation::NONE;
}
} // namespace

namespace ark {
const unsigned int EXTRA_THREAD_COUNT = 3; // main thread and message thread
const unsigned int HARDWARE_CONCURRENCY_COUNT = std::thread::hardware_concurrency();
const unsigned int MAX_THREAD_COUNT = HARDWARE_CONCURRENCY_COUNT > EXTRA_THREAD_COUNT ?
                                      HARDWARE_CONCURRENCY_COUNT - EXTRA_THREAD_COUNT : 1;
const unsigned int PROPER_THREAD_COUNT = MAX_THREAD_COUNT == 1 ? MAX_THREAD_COUNT : (MAX_THREAD_COUNT >> 1);
const int LSP_ERROR_CODE = 503;
CompilerCangjieProject *CompilerCangjieProject::instance = nullptr;
bool CompilerCangjieProject::useDB = false;

CompilerCangjieProject::CompilerCangjieProject(Callbacks *cb, lsp::IndexDatabase *arkIndexDB) : callback(cb)
{
    if (useDB) {
        auto bgIndexDB = std::make_unique<lsp::BackgroundIndexDB>(*arkIndexDB);
        backgroundIndexDb = std::move(bgIndexDB);
    }
    InitLRU();
}

PkgInfo::PkgInfo(const std::string &pkgPath,
                 const std::string &curModulePath,
                 const std::string &curModuleName,
                 Callbacks *callback)
{
    // init diag & diagTrash
    diag = std::make_unique<DiagnosticEngine>();
    std::unique_ptr<LSPDiagObserver> diagObserver = std::make_unique<LSPDiagObserver>(callback, *diag);
    diag->RegisterHandler(std::move(diagObserver));

    diagTrash = std::make_unique<DiagnosticEngine>();
    std::unique_ptr<LSPDiagObserver> diagTrashObserver = std::make_unique<LSPDiagObserver>(callback, *diagTrash);
    diagTrash->RegisterHandler(std::move(diagTrashObserver));

    std::string moduleSrcPath  = CompilerCangjieProject::GetInstance()->GetModuleSrcPath(curModulePath);
    packagePath = pkgPath;
    if (!curModulePath.empty()) {
        packageName = GetRealPkgNameFromPath(
            GetPkgNameFromRelativePath(GetRelativePath(moduleSrcPath, pkgPath) | IdenticalFunc));
        modulePath = curModulePath;
        moduleName = curModuleName;
    }

    compilerInvocation = std::make_unique<CompilerInvocation>();
    if (compilerInvocation) {
        compilerInvocation->globalOptions.moduleName = moduleName;
        compilerInvocation->globalOptions.compilePackage = true;
        compilerInvocation->globalOptions.packagePaths.push_back(pkgPath);
        compilerInvocation->globalOptions.outputMode = GlobalOptions::OutputMode::STATIC_LIB;
        const auto targetPackageName = packageName == "default" ? moduleName : moduleName + "." + packageName;
        compilerInvocation->globalOptions.passedWhenKeyValue = CompilerCangjieProject::GetInstance()->
                                                               GetConditionCompile(targetPackageName, moduleName);
        compilerInvocation->globalOptions.passedWhenCfgPaths = CompilerCangjieProject::GetInstance()->
                                                               GetConditionCompilePaths();
#ifdef _WIN32
        compilerInvocation->globalOptions.target.os = Cangjie::Triple::OSType::WINDOWS;
#elif __linux__
        compilerInvocation->globalOptions.target.os = Cangjie::Triple::OSType::LINUX;
#elif __APPLE__
        compilerInvocation->globalOptions.target.os = Cangjie::Triple::OSType::DARWIN;
#endif
        compilerInvocation->globalOptions.enableAddCommentToAst = true;
        // 2023/05/22 add macro expand with dynamic link library
        compilerInvocation->globalOptions.enableMacroInLSP = true;
        compilerInvocation->globalOptions.macroLib = CompilerCangjieProject::GetInstance()->GetMacroLibs();
        compilerInvocation->globalOptions.executablePath = CompilerCangjieProject::GetInstance()->GetCjc();
        auto dirs = GetDirectories(pkgPath);
        bool hasSubPkg = false;
        for (auto &dir : dirs) {
            auto files = GetAllFilesUnderCurrentPath(dir.path, CANGJIE_FILE_EXTENSION);
            if (!files.empty()) {
                hasSubPkg = true;
                break;
            }
        }
        compilerInvocation->globalOptions.noSubPkg = !hasSubPkg;
        // for deveco, default aarch64-linux-ohos
        if (!Options::GetInstance().IsOptionSet("test") && MessageHeaderEndOfLine::GetIsDeveco()) {
            compilerInvocation->globalOptions.target.arch = Triple::ArchType::AARCH64;
            compilerInvocation->globalOptions.target.os = Triple::OSType::LINUX;
            compilerInvocation->globalOptions.target.env = Triple::Environment::OHOS;
        }
    }
}

void CompilerCangjieProject::ClearCacheForDelete(const std::string &fullPkgName, const std::string &dirPath,
                                                 bool isInModule)
{
    if (isInModule) {
        (void) this->fullPkgNameToPath.erase(pathToFullPkgName[dirPath]);
        this->pathToFullPkgName.erase(dirPath);
        this->pkgInfoMap.erase(fullPkgName);
        // delete CIMap[pkgName]
        if (pLRUCache->HasCache(fullPkgName)) {
            for (auto &iter : pLRUCache->Get(fullPkgName)->upstreamPkgs) {
                LSPCompilerInstance::dependentPackageMap[iter].downstreamPkgs.erase(fullPkgName);
            }
        }
        this->pLRUCache->EraseCache(fullPkgName);
        this->CIMap.erase(fullPkgName);
    } else {
        this->pkgInfoMapNotInSrc.erase(dirPath);
        // delete CIMapNotInSrc[pkgName]
        if (pLRUCache->HasCache(dirPath)) {
            for (auto &iter : pLRUCache->Get(dirPath)->upstreamPkgs) {
                LSPCompilerInstance::dependentPackageMap[iter].downstreamPkgs.erase(dirPath);
            }
        }
        this->pLRUCache->EraseCache(dirPath);
        this->CIMapNotInSrc.erase(dirPath);
    }
    LSPCompilerInstance::astDataMap.erase(fullPkgName);
    this->packageInstanceCache.erase(dirPath);
}

void CompilerCangjieProject::IncrementForFileDelete(const std::string &fileName)
{
    // fix linux delete file crash
    std::string absName = FileStore::NormalizePath(fileName);
    std::string dirPath = GetDirPath(absName);
    std::string fullPkgName = GetFullPkgName(absName);

    Ptr<Package> package;
    bool isInModule = GetCangjieFileKind(absName).first != CangjieFileKind::IN_PROJECT_NOT_IN_SOURCE;
    if ((isInModule && pkgInfoMap.find(fullPkgName) == pkgInfoMap.end()) ||
        (!isInModule && pkgInfoMapNotInSrc.find(dirPath) == pkgInfoMapNotInSrc.end())) {
        return;
    }
    if (isInModule) {
        pkgInfoMap[fullPkgName]->bufferCache.erase(absName);
        IncrementCompile(absName, "", true);
        fullPkgName = GetFullPkgName(absName);
        if (!pLRUCache->HasCache(fullPkgName)) {
            return;
        }
        package = pLRUCache->Get(fullPkgName)->GetSourcePackages()[0];
    } else {
        pkgInfoMapNotInSrc[dirPath]->bufferCache.erase(absName);
        IncrementCompileForFileNotInSrc(absName, "", true);
        if (!pLRUCache->HasCache(dirPath)) {
            return;
        }
        package = pLRUCache->Get(dirPath)->GetSourcePackages()[0];
    }
    if (package == nullptr) {
        return;
    }
    {
        std::unique_lock<std::recursive_mutex> lock(fileCacheMtx);
        fileCache.erase(fileName);
    }
    if (package->files.empty()) {
        ClearCacheForDelete(fullPkgName, dirPath, isInModule);
    }
}

std::string CompilerCangjieProject::GetFullPkgName(const std::string &filePath) const
{
    std::string normalizePath = Normalize(filePath);
    std::string dirPath = GetDirPath(normalizePath);
    if (pathToFullPkgName.find(dirPath) == pathToFullPkgName.end()) {
        return GetRealPkgNameFromPath(dirPath);
    }
    return pathToFullPkgName.at(dirPath);
}

void CompilerCangjieProject::IncrementCompile(const std::string &filePath, const std::string &contents, bool isDelete)
{
    // Create new compiler instance and set some required options.
    Trace::Log("Start incremental compilation for package: ", filePath);
    auto fullPkgName = GetFullPkgName(filePath);
    // Remove diagnostics for the current package
    callback->RemoveDiagOfCurPkg(pkgInfoMap[fullPkgName]->packagePath);
    // Construct a compiler instance for compilation.
    auto &invocation = pkgInfoMap[fullPkgName]->compilerInvocation;
    auto &diag = pkgInfoMap[fullPkgName]->diag;
    auto ci = std::make_unique<LSPCompilerInstance>(callback, *invocation, *diag, fullPkgName, moduleManager);
    ci->cangjieHome = modulesHome;
    ci->loadSrcFilesFromCache = true;
    if (!isDelete && !Cangjie::FileUtil::HasExtension(filePath, CANGJIE_MACRO_FILE_EXTENSION)) {
        if (filePath.find(pkgInfoMap[fullPkgName]->modulePath) != std::string::npos) {
            std::lock_guard<std::mutex> lock(pkgInfoMap[fullPkgName]->pkgInfoMutex);
            pkgInfoMap[fullPkgName]->bufferCache[filePath] = contents;
        }
    }

    // 1. parse and update dependency
    if (!UpdateDependencies(fullPkgName, ci)) {
        return;
    }
    // detect circular dependency
    auto cycles = graph->FindCycles();
    if (!cycles.second) {
        // 2. check whether the upstream package cjo is fresh.
        auto upPackages = graph->FindAllDependencies(fullPkgName);
        // Submit tasks to Thread Pool Compilation
        auto recompileTasks = cjoManager->CheckStatus(upPackages);
        SubmitTasksToPool(recompileTasks);
    }
    // 3. compile current package
    bool changed = ci->CompileAfterParse(cjoManager, graph);
    // Updates the status of the downstream package cjo cache.
    if (changed) {
        cjoManager->UpdateDownstreamPackages(fullPkgName, graph);
    }

    // 4. build symbol index
    BuildIndex(ci);
    auto ret = InitCache(ci, fullPkgName, true);
    if (!ret) {
        Trace::Elog("InitCache Failed");
    }
    // 5. set LRUCache
    pLRUCache->Set(fullPkgName, ci);
    if (cycles.second) {
        ReportCircularDeps(cycles.first);
    }
    Trace::Log("Finish incremental compilation for package: ", fullPkgName);
}

bool CompilerCangjieProject::UpdateDependencies(
    std::string &fullPkgName, const std::unique_ptr<LSPCompilerInstance> &ci)
{
    {
        std::lock_guard<std::mutex> lock(pkgInfoMap[fullPkgName]->pkgInfoMutex);
        ci->bufferCache = pkgInfoMap[fullPkgName]->bufferCache;
    }
    ci->PreCompileProcess();
    auto packages = ci->GetSourcePackages();
    if (packages.empty() || !packages[0]) {
        return false;
    }
    std::string pkgName;
    // Files is empty when you delete all files in package.
    if (packages[0]->files.empty()) {
        pkgName = fullPkgName;
    } else {
        pkgName = moduleManager->GetExpectedPkgName(*packages[0]->files[0]);
    }
    bool redefined = false;
    std::lock_guard<std::mutex> lock(pkgInfoMap[fullPkgName]->pkgInfoMutex);
    if (pkgInfoMap[fullPkgName]->isSourceDir && pkgName != fullPkgName) {
        if (pkgInfoMap.find(pkgName) != pkgInfoMap.end()) {
            redefined = true;
        } else {
            pathToFullPkgName[pkgInfoMap[fullPkgName]->packagePath] = pkgName;
            fullPkgNameToPath[pkgName] = pkgInfoMap[fullPkgName]->packagePath;
            pkgInfoMap[fullPkgName]->packageName = SplitFullPackage(pkgName).second;
            pkgInfoMap[pkgName] = std::move(pkgInfoMap[fullPkgName]);
            pkgInfoMap.erase(fullPkgName);
            (void) fullPkgNameToPath.erase(fullPkgName);
            pLRUCache->EraseCache(fullPkgName);
            CIMap.erase(fullPkgName);
            LSPCompilerInstance::astDataMap.erase(fullPkgName);
            ci->pkgNameForPath = pkgName;
            fullPkgName = pkgName;
        }
    }
    ci->UpdateDepGraph(graph, fullPkgName);
    if (!ci->GetSourcePackages()[0]->files.empty()) {
        if (ci->GetSourcePackages()[0]->files[0]->package) {
            auto mod = GetPackageSpecMod(ci->GetSourcePackages()[0]->files[0]->package.get());
            (void)pkgToModMap.insert_or_assign(fullPkgName, mod);
        }
    }
    if (!redefined) {
        for (const auto &file : packages[0]->files) {
            CheckPackageNameByAbsName(*file, fullPkgName);
        }
    }
    return true;
}

void CompilerCangjieProject::SubmitTasksToPool(const std::unordered_set<std::string> &tasks)
{
    if (tasks.empty()) {
        return;
    }
    auto allTasks{tasks};
    std::unordered_set<std::string> outsideTasks{};
    std::unordered_map<std::string, std::unordered_set<uint64_t>> dependencies;
    for (auto &package : tasks) {
        auto allDependencies = graph->FindAllDependencies(package);
        for (const auto &it : cjoManager->CheckStatus(allDependencies)) {
            if (tasks.find(it) == tasks.end()) {
                outsideTasks.insert(it);
            }
            dependencies[package].emplace(GenTaskId(it));
        }
    }
    for (auto &package: outsideTasks) {
        auto allDependencies = graph->FindAllDependencies(package);
        for (const auto &it : cjoManager->CheckStatus(allDependencies)) {
            dependencies[package].emplace(GenTaskId(it));
        }
    }
    allTasks.insert(outsideTasks.begin(), outsideTasks.end());

    auto sortedTasks = graph->PartialTopologicalSort(allTasks, true);
    for (auto &package: sortedTasks) {
        auto taskId = GenTaskId(package);
        auto task = [this, taskId, package]() {
            Trace::Log("start execute task", package);
            if (cjoManager->GetStatus(package) !=  DataStatus::STALE) {
                Trace::Log("Do not need to recompile package", package);
                cjoManager->UpdateStatus({package}, DataStatus::FRESH);
                thrdPool->TaskCompleted(taskId);
                Trace::Log("finish execute task", package);
                return;
            }
            callback->RemoveDiagOfCurPkg(pkgInfoMap[package]->packagePath);
            auto &invocation = pkgInfoMap[package]->compilerInvocation;
            auto &diag = pkgInfoMap[package]->diag;
            auto ci = std::make_unique<LSPCompilerInstance>(callback, *invocation, *diag, package, moduleManager);
            ci->cangjieHome = modulesHome;
            ci->loadSrcFilesFromCache = true;
            ci->bufferCache = pkgInfoMap[package]->bufferCache;
            ci->PreCompileProcess();
            bool changed = ci->CompileAfterParse(cjoManager, graph);
            if (changed) {
                Trace::Log("cjo has changed, need to update down stream packages status", package);
                cjoManager->UpdateDownstreamPackages(package, graph);
            }
            this->BuildIndex(ci);
            auto ret = InitCache(ci, package);
            if (!ret) {
                Trace::Elog("InitCache Failed");
            }
            pLRUCache->Set(package, ci);
            thrdPool->TaskCompleted(taskId);
            Trace::Log("finish execute task", package);
        };
        thrdPool->AddTask(taskId, dependencies[package], task);
    }
    thrdPool->WaitUntilAllTasksComplete();
}

void CompilerCangjieProject::IncrementOnePkgCompile(const std::string &filePath, const std::string &contents)
{
    std::string fullPkgName = GetFullPkgName(filePath);
    if (pkgInfoMap.find(fullPkgName) == pkgInfoMap.end()) {
        if (pkgInfoMapNotInSrc.find(fullPkgName) == pkgInfoMapNotInSrc.end()) {
            return;
        }
        if (!Cangjie::FileUtil::HasExtension(filePath, CANGJIE_MACRO_FILE_EXTENSION)) {
            std::lock_guard<std::mutex> lock(pkgInfoMapNotInSrc[fullPkgName]->pkgInfoMutex);
            pkgInfoMapNotInSrc[fullPkgName]->bufferCache[filePath] = contents;
        }
        IncrementTempPkgCompileNotInSrc(fullPkgName);
        return;
    }
    if (!Cangjie::FileUtil::HasExtension(filePath, CANGJIE_MACRO_FILE_EXTENSION)) {
        std::lock_guard<std::mutex> lock(pkgInfoMap[fullPkgName]->pkgInfoMutex);
        pkgInfoMap[fullPkgName]->bufferCache[filePath] = contents;
    }
    IncrementCompile(filePath, contents);
}

void CompilerCangjieProject::IncrementTempPkgCompile(const std::string &basicString)
{
    std::string fullPkgName = basicString;
    if (pkgInfoMap.find(basicString) == pkgInfoMap.end()) {
        return;
    }
    callback->RemoveDiagOfCurPkg(pkgInfoMap[fullPkgName]->packagePath);
    auto newCI = std::make_unique<LSPCompilerInstance>(callback, *pkgInfoMap[fullPkgName]->compilerInvocation,
        *pkgInfoMap[fullPkgName]->diag, fullPkgName, moduleManager);
    if (newCI == nullptr) {
        return;
    }
    newCI->cangjieHome = modulesHome;
    // update cache and read code from cache
    newCI->loadSrcFilesFromCache = true;
    if (!UpdateDependencies(fullPkgName, newCI)) {
        return;
    }
    newCI->CompileAfterParse(cjoManager, graph);
    BuildIndex(newCI);
    InitCache(newCI, fullPkgName);
    pLRUCache->Set(fullPkgName, newCI);
}

void CompilerCangjieProject::IncrementTempPkgCompileNotInSrc(const std::string &fullPkgName)
{
    std::string dirPath = fullPkgName;
    if (pkgInfoMapNotInSrc.find(dirPath) == pkgInfoMapNotInSrc.end()) {
        return;
    }
    callback->RemoveDiagOfCurPkg(pkgInfoMapNotInSrc[fullPkgName]->packagePath);
    auto newCI = std::make_unique<LSPCompilerInstance>(callback, *pkgInfoMapNotInSrc[dirPath]->compilerInvocation,
        *pkgInfoMapNotInSrc[dirPath]->diag, "", moduleManager);
    newCI->cangjieHome = modulesHome;
    newCI->loadSrcFilesFromCache = true;

    if (!ParseAndUpdateNotInSrcDep(dirPath, newCI)) {
        return;
    }
    newCI->CompileAfterParse(cjoManager, graph);
    BuildIndex(newCI);
    InitCache(newCI, fullPkgName, false);
    pLRUCache->Set(fullPkgName, newCI);
}

void CompilerCangjieProject::IncrementCompileForFileNotInSrc(const std::string &filePath, const std::string &contents,
                                                             bool isDelete)
{
    std::string dirPath = Normalize(GetDirPath(filePath));
    if (pkgInfoMapNotInSrc.find(dirPath) == pkgInfoMapNotInSrc.end()) {
        return;
    }
    callback->RemoveDiagOfCurPkg(pkgInfoMapNotInSrc[dirPath]->packagePath);
    auto newCI = std::make_unique<LSPCompilerInstance>(callback, *pkgInfoMapNotInSrc[dirPath]->compilerInvocation,
                                                       *pkgInfoMapNotInSrc[dirPath]->diag, "", moduleManager);
    newCI->cangjieHome = modulesHome;
    // update cache and read code from cache
    newCI->loadSrcFilesFromCache = true;
    if (!isDelete && !Cangjie::FileUtil::HasExtension(filePath, CANGJIE_MACRO_FILE_EXTENSION)) {
        pkgInfoMapNotInSrc[dirPath]->bufferCache[filePath] = contents;
    }
    if (!ParseAndUpdateNotInSrcDep(dirPath, newCI)) {
        return;
    }
    newCI->CompileAfterParse(cjoManager, graph);
    BuildIndex(newCI);
    InitCache(newCI, dirPath);
    pLRUCache->Set(dirPath, newCI);
}

bool CompilerCangjieProject::ParseAndUpdateNotInSrcDep(const std::string &dirPath,
                                                       const std::unique_ptr <LSPCompilerInstance> &newCI)
{
    {
        std::lock_guard<std::mutex> lock(pkgInfoMapNotInSrc[dirPath]->pkgInfoMutex);
        newCI->bufferCache = pkgInfoMapNotInSrc[dirPath]->bufferCache;
    }
    newCI->PreCompileProcess();
    newCI->UpdateDepGraph();
    auto packages = newCI->GetSourcePackages();
    if (packages.empty() || !packages[0]) {
        return false;
    }
    std::string expectedPkgName = packages[0]->fullPackageName;
    for (const auto &file : packages[0]->files) {
        auto userWrittenPackage = file->package == nullptr ? DEFAULT_PACKAGE_NAME : file->curPackage->fullPackageName;
        if (userWrittenPackage != expectedPkgName) {
            auto errPos = getPackageNameErrPos(*file);
            pkgInfoMapNotInSrc[dirPath]->diag->DiagnoseRefactor(DiagKindRefactor::package_name_not_identical_lsp,
                                                                errPos,
                                                                expectedPkgName);
        }
    }
    return true;
}

void CompilerCangjieProject::GetIncDegree(const std::string &pkgName,
                                          std::unordered_map<std::string, size_t> &inDegreeMap,
                                          std::unordered_map<std::string, bool> &isVisited)
{
    if (!LSPCompilerInstance::dependentPackageMap.count(pkgName)) {
        return;
    }
    for (auto &dep : LSPCompilerInstance::dependentPackageMap[pkgName].downstreamPkgs) {
        inDegreeMap[dep]++;
        auto iter = isVisited.find(dep);
        if (iter == isVisited.end() || !isVisited[dep]) {
            isVisited[dep] = true;
            GetIncDegree(dep, inDegreeMap, isVisited);
        }
    }
}

std::vector<std::string> CompilerCangjieProject::GetIncTopologySort(const std::string &pkgName)
{
    std::unordered_map<std::string, size_t> inDegreeMap;
    std::unordered_map<std::string, bool> isVisited;
    GetIncDegree(pkgName, inDegreeMap, isVisited);
    std::queue<std::string> que;
    std::vector<std::string> sortResult;
    que.emplace(pkgName);
    while (!que.empty()) {
        std::string tmpName = que.front();
        sortResult.push_back(tmpName);
        que.pop();
        if (!LSPCompilerInstance::dependentPackageMap.count(tmpName)) {
            continue;
        }
        for (auto &outEdge : LSPCompilerInstance::dependentPackageMap[tmpName].downstreamPkgs) {
            inDegreeMap[outEdge]--;
            if (inDegreeMap[outEdge] == 0) {
                que.emplace(outEdge);
            }
        }
    }
    return sortResult;
}

void CompilerCangjieProject::CompilerOneFile(
    const std::string &file, const std::string &contents, Position pos, bool onlyParse, const std::string &name)
{
    std::string absName = Normalize(file);
    Trace::Log("Start analyzing the file: ", absName);

    std::string dirPath = GetDirPath(absName);
    std::string fullPkgName = GetFullPkgName(absName);
    auto [fileKind, modulePath] = GetCangjieFileKind(absName);
    if (onlyParse) {
        // for normalComplete
        if (fileKind == CangjieFileKind::IN_PROJECT_NOT_IN_SOURCE) {
            IncrementCompileForCompleteNotInSrc(name, absName, contents);
        } else {
            IncrementCompileForComplete(name, absName, pos, contents);
        }
        return;
    }

    if (fileKind == CangjieFileKind::IN_PROJECT_NOT_IN_SOURCE) {
        // for file not in src
        if (pkgInfoMapNotInSrc.find(dirPath) == pkgInfoMapNotInSrc.end()) {
            pkgInfoMapNotInSrc[dirPath] = std::make_unique<PkgInfo>(dirPath, "", "", callback);
        }
        if (!Cangjie::FileUtil::HasExtension(absName, CANGJIE_MACRO_FILE_EXTENSION)) {
            std::lock_guard<std::mutex> lock(pkgInfoMapNotInSrc[dirPath]->pkgInfoMutex);
            pkgInfoMapNotInSrc[dirPath]->bufferCache[absName] = contents;
        }
        IncrementOnePkgCompile(absName, contents);
    } else if (fileKind == CangjieFileKind::IN_NEW_PACKAGE) {
        // in new package in src
        auto moduleInfo = moduleManager->moduleInfoMap[modulePath];
        std::string moduleName = moduleInfo.moduleName;
        std::string sourcePath = GetModuleSrcPath(moduleInfo.modulePath);
        auto relativePath = GetRelativePath(sourcePath, dirPath);
        std::string pkgName = GetRealPkgNameFromPath(GetPkgNameFromRelativePath(relativePath | IdenticalFunc));
        if (pkgName == "default" && (!relativePath.has_value() || relativePath.value().empty())) {
            fullPkgName = moduleName;
        } else {
            fullPkgName = moduleName + "." + pkgName;
        }
        bool invalid = pkgInfoMap.find(fullPkgName) != pkgInfoMap.end() && pkgInfoMap[fullPkgName]->isSourceDir &&
                       pLRUCache->HasCache(fullPkgName) && pLRUCache->Get(fullPkgName) != nullptr;
        if (invalid) {
            std::string defaultPkgName = DEFAULT_PACKAGE_NAME;
            pkgInfoMap[defaultPkgName] = std::move(pkgInfoMap[fullPkgName]);
            auto setRes = pLRUCache->Set(defaultPkgName, pLRUCache->Get(fullPkgName));
            EraseOtherCache(setRes);
            pLRUCache->EraseCache(fullPkgName);
            CIMap.erase(fullPkgName);
            pathToFullPkgName[sourcePath] = defaultPkgName;
            fullPkgNameToPath[defaultPkgName] = sourcePath;
        }
        pkgInfoMap[fullPkgName] =
            std::make_unique<PkgInfo>(dirPath, moduleInfo.modulePath, moduleInfo.moduleName, callback);
        auto found = fullPkgName.find_last_of(DOT);
        if (found != std::string::npos) {
            auto subPkgName = fullPkgName.substr(0, found);
            if (pkgInfoMap.find(subPkgName) != pkgInfoMap.end() &&
                pkgInfoMap[subPkgName]->compilerInvocation->globalOptions.noSubPkg) {
                pkgInfoMap[subPkgName]->compilerInvocation->globalOptions.noSubPkg = false;
                cjoManager->UpdateStatus({subPkgName}, DataStatus::STALE);
            }
        }
        if (!Cangjie::FileUtil::HasExtension(absName, CANGJIE_MACRO_FILE_EXTENSION)) {
            pkgInfoMap[fullPkgName]->bufferCache[absName] = contents;
        }
        if (pkgName == DEFAULT_PACKAGE_NAME) {
            pkgInfoMap[fullPkgName]->isSourceDir = true;
        }
        pathToFullPkgName[dirPath] = fullPkgName;
        fullPkgNameToPath[fullPkgName] = dirPath;
        CIMap.insert_or_assign(fullPkgName, nullptr);
        IncrementCompile(absName, contents);
    } else {
        // in old package in src
        IncrementCompile(absName, contents);
    }
    Trace::Log("Finish analyzing the file: ", absName);
}

void CompilerCangjieProject::IncrementCompileForComplete(
    const std::string &name, const std::string &filePath, Position pos, const std::string &contents)
{
    std::string pkgName = GetFullPkgName(filePath);
    // delete and add new CI
    auto newCI = std::make_unique<LSPCompilerInstance>(
        callback, *pkgInfoMap[pkgName]->compilerInvocation, *pkgInfoMap[pkgName]->diagTrash, pkgName, moduleManager);

    if (newCI == nullptr) {
        return;
    }
    newCI->cangjieHome = modulesHome;
    // update cache and read code from cache
    newCI->loadSrcFilesFromCache = true;
    if (pkgInfoMap[pkgName]->bufferCache.count(filePath) &&
        !Cangjie::FileUtil::HasExtension(filePath, CANGJIE_MACRO_FILE_EXTENSION)) {
        std::lock_guard<std::mutex> lock(pkgInfoMap[pkgName]->pkgInfoMutex);
        pkgInfoMap[pkgName]->bufferCache[filePath] = contents;
    }
    newCI->bufferCache = pkgInfoMap[pkgName]->bufferCache;
    newCI->CompilePassForComplete(cjoManager, graph, pos, name);
    CIForParse = std::move(newCI);
    InitParseCache(CIForParse, pkgName);
}

std::unique_ptr<LSPCompilerInstance> CompilerCangjieProject::GetCIForDotComplete(
    const std::string &filePath, Position pos, std::string &contents)
{
    std::string pkgName = GetFullPkgName(filePath);
    auto newCI = std::make_unique<LSPCompilerInstance>(
        callback, *pkgInfoMap[pkgName]->compilerInvocation, *pkgInfoMap[pkgName]->diagTrash,
        pkgName, moduleManager);

    if (!newCI) {
        return nullptr;
    }
    newCI->cangjieHome = modulesHome;
    newCI->loadSrcFilesFromCache = true;
    // Get the file content before enter "."
    if (!DeleteCharForPosition(contents, pos.line, pos.column - 1)) {
        return nullptr;
    }

    newCI->bufferCache = pkgInfoMap[pkgName]->bufferCache;
    if (newCI->bufferCache.count(filePath) &&
        !Cangjie::FileUtil::HasExtension(filePath, CANGJIE_MACRO_FILE_EXTENSION)) {
        newCI->bufferCache[filePath] = contents;
    }
    newCI->CompilePassForComplete(cjoManager, graph, pos);
    newCI->CompileAfterParse(cjoManager, graph);

    return newCI;
}

void CompilerCangjieProject::IncrementCompileForCompleteNotInSrc(
    const std::string &name, const std::string &filePath, const std::string &contents)
{
    std::string dirPath = Normalize(GetDirPath(filePath));
    if (pkgInfoMapNotInSrc.find(dirPath) == pkgInfoMapNotInSrc.end()) {
        return;
    }

    auto newCI = std::make_unique<LSPCompilerInstance>(
        callback, *pkgInfoMapNotInSrc[dirPath]->compilerInvocation,
        *pkgInfoMapNotInSrc[dirPath]->diagTrash, "", moduleManager);
    if (newCI == nullptr) {
        return;
    }
    newCI->cangjieHome = modulesHome;
    // update cache and read code from cache
    newCI->loadSrcFilesFromCache = true;
    if (pkgInfoMapNotInSrc[dirPath]->bufferCache.count(filePath)) {
        pkgInfoMapNotInSrc[dirPath]->bufferCache[filePath] = contents;
    }
    newCI->bufferCache = pkgInfoMapNotInSrc[dirPath]->bufferCache;

    newCI->CompilePassForComplete(cjoManager, graph, INVALID_POSITION, name);
    CIForParse = std::move(newCI);
    InitParseCache(CIForParse, "");
}

void CompilerCangjieProject::InitParseCache(const std::unique_ptr<LSPCompilerInstance> &lspCI,
                                            const std::string &pkgForPath)
{
    for (auto pkg : lspCI->GetSourcePackages()) {
        auto pkgInstance = std::make_unique<PackageInstance>(lspCI->diag, lspCI->importManager);
        pkgInstance->package = pkg;
        pkgInstance->ctx = nullptr;
        this->packageInstanceCacheForParse = std::move(pkgInstance);
        for (auto &file : pkg->files) {
            std::string contents;
            if (!pkgForPath.empty()) {
                if (pkgInfoMap.find(pkgForPath) == pkgInfoMap.end()) {
                    continue;
                }
                std::lock_guard<std::mutex> lock(pkgInfoMap[pkgForPath]->pkgInfoMutex);
                contents = pkgInfoMap[pkgForPath]->bufferCache[file->filePath];
            } else {
                std::string dirPath = Normalize(GetDirPath(file->filePath));
                if (pkgInfoMapNotInSrc.find(dirPath) == pkgInfoMapNotInSrc.end()) {
                    continue;
                }
                std::lock_guard<std::mutex> lock(pkgInfoMapNotInSrc[dirPath]->pkgInfoMutex);
                contents = pkgInfoMapNotInSrc[dirPath]->bufferCache[file->filePath];
            }
            std::pair<std::string, std::string> paths = { file->filePath, contents };
            auto arkAST = std::make_unique<ArkAST>(paths, file.get(), lspCI->diag,
                                                   this->packageInstanceCacheForParse.get(),
                                                   &lspCI->GetSourceManager());
            std::string absName = FileStore::NormalizePath(file->filePath);
            int fileId = lspCI->GetSourceManager().GetFileID(absName);
            if (fileId >= 0) {
                arkAST->fileID = static_cast<unsigned int>(fileId);
            }
            {
                std::unique_lock<std::mutex> lock(fileMtx);
                this->fileCacheForParse[absName] = std::move(arkAST);
            }
        }
    }
}

std::pair<CangjieFileKind, std::string> CompilerCangjieProject::GetCangjieFileKind(const std::string &filePath) const
{
    std::string normalizeFilePath = Normalize(filePath);
    std::string dirPath = GetDirPath(normalizeFilePath);
    if (pathToFullPkgName.find(dirPath) != pathToFullPkgName.end()) {
        return {CangjieFileKind::IN_OLD_PACKAGE, pkgInfoMap.at(pathToFullPkgName.at(dirPath))->modulePath};
    }
    normalizeFilePath = normalizeFilePath.empty() ? "" : JoinPath(normalizeFilePath, "");
    std::string normalizedSourcePath;
    for (const auto &item : moduleManager->moduleInfoMap) {
        normalizedSourcePath = item.first.empty() ? "" : GetInstance()->GetModuleSrcPath(item.first);
        if (IsUnderPath(normalizedSourcePath, normalizeFilePath)) {
            return {CangjieFileKind::IN_NEW_PACKAGE, item.first};
        }
    }

    if (dirPath.find(stdLibPath) != std::string::npos) {
        return {CangjieFileKind::IN_PROJECT_NOT_IN_SOURCE, dirPath};
    }

    if (dirPath == moduleManager->projectRootPath) {
        return {CangjieFileKind::IN_PROJECT_NOT_IN_SOURCE, dirPath};
    }

    if (MessageHeaderEndOfLine::GetIsDeveco()) {
        for (const auto &item : moduleManager->moduleInfoMap) {
            if (dirPath == item.first) {
                return {CangjieFileKind::IN_PROJECT_NOT_IN_SOURCE, dirPath};
            }
        }
    }

    return {CangjieFileKind::MISSING, ""};
}

bool CompilerCangjieProject::InitCache(const std::unique_ptr<LSPCompilerInstance> &lspCI, const std::string &pkgForPath,
                                       bool isInModule)
{
    for (auto pkg : lspCI->GetSourcePackages()) {
        auto pkgInstance = std::make_unique<PackageInstance>(lspCI->diag, lspCI->importManager);
        pkgInstance->package = pkg;
        auto ctx = lspCI->GetASTContextByPackage(pkg);
        if (ctx == nullptr) {
            Logger::Instance().LogMessage(MessageType::MSG_ERROR, "invoke kernel GetASTContextByPackage fail!");
            return false;
        }
        pkgInstance->ctx = ctx;
        if (pkg->files.empty()) {
            return true;
        }

        // if pkg->files[0]->filePath is a dir file we need dirPath is pkgForPath
        std::string dirPath = Normalize(GetDirPath(pkg->files[0]->filePath));
        if (GetFileExtension(pkg->files[0]->filePath) != "cj") {
            dirPath = Normalize(pkg->files[0]->filePath);
        }

        {
            std::unique_lock lock(fileCacheMtx);
            this->packageInstanceCache[dirPath] = std::move(pkgInstance);
        }

        for (auto &file : pkg->files) {
            auto filePath = file->filePath;
            // filePath maybe a dir not a file
            if (GetFileExtension(filePath) != "cj") { continue; }
            LowFileName(filePath);
            std::string contents;
            if (isInModule) {
                if (pkgInfoMap.find(pkgForPath) == pkgInfoMap.end()) {
                    continue;
                }
                {
                    std::lock_guard<std::mutex> lock(pkgInfoMap[pkgForPath]->pkgInfoMutex);
                    contents = pkgInfoMap[pkgForPath]->bufferCache[filePath];
                }
            } else {
                if (pkgInfoMapNotInSrc.find(dirPath) == pkgInfoMapNotInSrc.end()) {
                    continue;
                }
                {
                    std::lock_guard<std::mutex> lock(pkgInfoMapNotInSrc[pkgForPath]->pkgInfoMutex);
                    contents = pkgInfoMapNotInSrc[dirPath]->bufferCache[filePath];
                }
            }
            std::pair<std::string, std::string> paths = {filePath, contents};
            auto arkAST = std::make_unique<ArkAST>(paths, file.get(), lspCI->diag, packageInstanceCache[dirPath].get(),
                                                   &lspCI->GetSourceManager());
            std::string absName = FileStore::NormalizePath(filePath);
            int fileId = lspCI->GetSourceManager().GetFileID(absName);
            if (fileId >= 0) {
                arkAST->fileID = static_cast<unsigned int>(fileId);
            }
            {
                std::unique_lock<std::recursive_mutex> lock(fileCacheMtx);
                this->fileCache[absName] = std::move(arkAST);
            }
        }
    }
    return true;
}

void CompilerCangjieProject::InitOneModule(const ModuleInfo &moduleInfo)
{
    std::string sourcePath = Normalize(GetModuleSrcPath(moduleInfo.modulePath));
    if (!FileExist(sourcePath)) {
        return;
    }
    std::string rootPackageName = moduleInfo.moduleName;

    pkgInfoMap[rootPackageName] =
        std::make_unique<PkgInfo>(sourcePath, moduleInfo.modulePath, moduleInfo.moduleName, callback);
    pkgInfoMap[rootPackageName]->isSourceDir = true;
    auto allFiles = GetAllFilesUnderCurrentPath(sourcePath, CANGJIE_FILE_EXTENSION, false);
    if (allFiles.empty()) {
        (void)pkgInfoMap.erase(rootPackageName);
    } else {
        this->pathToFullPkgName[sourcePath] = rootPackageName;
        this->fullPkgNameToPath[rootPackageName] = sourcePath;
        for (auto &file : allFiles) {
            auto filePath = NormalizePath(JoinPath(sourcePath, file));
            LowFileName(filePath);
            (void)pkgInfoMap[rootPackageName]->bufferCache.emplace(filePath, GetFileContents(filePath));
        }
    }
    for (auto &packagePath: GetAllDirsUnderCurrentPath(sourcePath)) {
        packagePath = Normalize(packagePath);
        std::string pkgName = GetRealPkgNameFromPath(
            GetPkgNameFromRelativePath(GetRelativePath(sourcePath, packagePath) |IdenticalFunc));
        std::string fullPackageName = SpliceFullPkgName(moduleInfo.moduleName, pkgName);
        pkgInfoMap[fullPackageName] = std::make_unique<PkgInfo>(packagePath, moduleInfo.modulePath,
                                                                moduleInfo.moduleName, callback);
        allFiles = GetAllFilesUnderCurrentPath(packagePath, CANGJIE_FILE_EXTENSION, false);
        if (allFiles.empty()) {
            pkgInfoMap.erase(fullPackageName);
            continue;
        }
        this->pathToFullPkgName[packagePath] = fullPackageName;
        this->fullPkgNameToPath[fullPackageName] = packagePath;
        for (auto &file : allFiles) {
            auto filePath = NormalizePath(JoinPath(packagePath, file));
            LowFileName(filePath);
            pkgInfoMap[fullPackageName]->bufferCache.emplace(filePath, GetFileContents(filePath));
        }
    }
}

void CompilerCangjieProject::InitNotInModule()
{
    LSPCompilerInstance::SetCjoPathInModules(modulesHome, cangjiePath);
    LSPCompilerInstance::InitCacheFileCacheMap();

    // set nosrc buffer cache
    std::vector<std::string> notInSrcDirs;
    if (MessageHeaderEndOfLine::GetIsDeveco()) {
        for (const auto &item : moduleManager->moduleInfoMap) {
            if (!item.first.empty()) {
                notInSrcDirs.emplace_back(item.first);
            }
        }
    } else {
        notInSrcDirs.emplace_back(moduleManager->projectRootPath);
    }

    for (const auto& nosrc : notInSrcDirs) {
        auto allFiles = GetAllFilesUnderCurrentPath(nosrc, CANGJIE_FILE_EXTENSION, false);
        pkgInfoMapNotInSrc[nosrc] = std::make_unique<PkgInfo>(nosrc, "", "", callback);
        for (const auto &file : allFiles) {
            std::string filePath = NormalizePath(JoinPath(nosrc, file));
            LowFileName(filePath);
            pkgInfoMapNotInSrc[nosrc]->bufferCache.emplace(filePath, GetFileContents(filePath));
        }
    }

    // set stdlib buffer cache
    auto cjLibDirs = GetAllDirsUnderCurrentPath(stdLibPath);

    for (const auto &cjLib : cjLibDirs) {
        auto allFiles = GetAllFilesUnderCurrentPath(cjLib, CANGJIE_FILE_EXTENSION);
        pkgInfoMapNotInSrc[cjLib] = std::make_unique<PkgInfo>(cjLib, "", "", callback);
        std::vector<std::string> nativeFiles;
        CategorizeFiles(allFiles, nativeFiles);
        for (const auto &file : nativeFiles) {
            std::string filePath = NormalizePath(JoinPath(cjLib, file));
            LowFileName(filePath);
            pkgInfoMapNotInSrc[cjLib]->bufferCache.emplace(filePath, GetFileContents(filePath));
        }
    }
}

void CompilerCangjieProject::UpdateDownstreamPackages()
{
    for (auto &dep : LSPCompilerInstance::dependentPackageMap) {
        std::vector<std::string> willDeleteKey;
        for (auto &item : dep.second.importPackages) {
            if (!LSPCompilerInstance::dependentPackageMap.count(item) &&
                LSPCompilerInstance::dependentPackageMap[dep.first].importPackages.count(item)) {
                willDeleteKey.push_back(item);
                continue;
            }
            LSPCompilerInstance::dependentPackageMap[item].downstreamPkgs.insert(dep.first);
        }
        for (auto &item : willDeleteKey) {
            dep.second.importPackages.erase(item);
        }
        dep.second.inDegree = dep.second.importPackages.size();
    }
}

void CompilerCangjieProject::InitPkgInfoAndParseInModule()
{
    for (auto &item : pkgInfoMap) {
        auto pkgCompiler = std::make_unique<LSPCompilerInstance>(
            callback, *item.second->compilerInvocation, *item.second->diag, item.first, moduleManager);
        if (pkgCompiler == nullptr) {
            return;
        }
        for (auto &file : item.second->bufferCache) {
            auto fileName = file.first;
            this->callback->AddDocWhenInitCompile(FileStore::NormalizePath(fileName));
        }
        pkgCompiler->cangjieHome = modulesHome;
        pkgCompiler->loadSrcFilesFromCache = true;
        pkgCompiler->bufferCache = item.second->bufferCache;
        pkgCompiler->PreCompileProcess();
        CjoData cjoData;
        cjoData.data = {};
        cjoData.status = DataStatus::STALE;
        cjoManager->SetData(item.first, cjoData);
        auto packages = pkgCompiler->GetSourcePackages();
        if (packages.empty() || !packages[0]) {
            return;
        }
        std::string fullPackageName = item.first;
        pkgCompiler->UpdateDepGraph(graph, item.first);
        if (!pkgCompiler->GetSourcePackages()[0]->files.empty()) {
            if (pkgCompiler->GetSourcePackages()[0]->files[0]->package) {
                auto mod = GetPackageSpecMod(pkgCompiler->GetSourcePackages()[0]->files[0]->package.get());
                (void)pkgToModMap.emplace(fullPackageName, mod);
            }
        }
        CIMap[fullPackageName] = std::move(pkgCompiler);
    }
    for (auto &item : CIMap) {
        for (const auto &file : item.second->GetSourcePackages()[0]->files) {
            CheckPackageNameByAbsName(*file, item.first);
        }
    }
}

void CompilerCangjieProject::InitPkgInfoAndParseNotInModule()
{
    for (const auto &item : pkgInfoMapNotInSrc) {
        auto parser = [this, &item]() {
            auto pkgCompiler = std::make_unique<LSPCompilerInstance>(
                callback, *item.second->compilerInvocation, *item.second->diag, "", moduleManager);
            if (pkgCompiler == nullptr) {
                return;
            }
            pkgCompiler->cangjieHome = modulesHome;
            pkgCompiler->loadSrcFilesFromCache = true;
            pkgCompiler->bufferCache = item.second->bufferCache;
            pkgCompiler->PreCompileProcess();
            pkgCompiler->UpdateDepGraph(false);
            auto packages = pkgCompiler->GetSourcePackages();
            if (packages.empty() || !packages[0]) {
                return;
            }
            std::string expectedPkgName = packages[0]->fullPackageName;
            for (const auto &file : packages[0]->files) {
                auto userWrittenPackage =
                    file->package == nullptr ? DEFAULT_PACKAGE_NAME : file->package->packageName;
                if (userWrittenPackage != expectedPkgName) {
                    auto errPos = getPackageNameErrPos(*file);
                    (void)item.second->diag->DiagnoseRefactor(
                        DiagKindRefactor::package_name_not_identical_lsp, errPos, expectedPkgName);
                }
            }
            CIMapNotInSrc[item.first] = std::move(pkgCompiler);
        };
        parser();
    }
}

void CompilerCangjieProject::InitPkgInfoAndParse()
{
    LSPCompilerInstance::SetCjoPathInModules(modulesHome, cangjiePath);
    LSPCompilerInstance::InitCacheFileCacheMap();

    for (auto &item : moduleManager->moduleInfoMap) {
        InitOneModule(item.second);
        LSPCompilerInstance::UpdateUsrCjoFileCacheMap(item.second.moduleName, item.second.cjoRequiresMap);
    }
    InitNotInModule();

    InitPkgInfoAndParseInModule();
    InitPkgInfoAndParseNotInModule();
    UpdateDownstreamPackages();
}

void CompilerCangjieProject::EraseOtherCache(const std::string &fullPkgName)
{
    if (fullPkgName.empty() || fullPkgNameToPath.find(fullPkgName) == fullPkgNameToPath.end() ||
        !(IsFromCIMap(fullPkgName) || PkgIsFromCIMapNotInSrc(fullPkgName))) {
        return;
    }
    auto dirPath = fullPkgNameToPath.at(fullPkgName);
    if (IsFromCIMap(fullPkgName)) {
        // erase fileCache and packageInstanceCache
        for (const auto &file : pkgInfoMap[fullPkgName]->bufferCache) {
            auto absPath = FileStore::NormalizePath(file.first);
            if (fileCache.find(absPath) == fileCache.end()) { continue; }
            {
                std::unique_lock<std::recursive_mutex> lock(fileCacheMtx);
                fileCache[absPath] = nullptr;
                fileCache.erase(absPath);
            }
        }
        packageInstanceCache.erase(dirPath);
#ifdef __linux__
        (void) malloc_trim(0);
#elif __APPLE__
        (void) malloc_zone_pressure_relief(malloc_default_zone(), 0);
#endif
        return;
    }
    //  PkgIsFromCIMapNotInSrc
    for (const auto &file : pkgInfoMapNotInSrc[fullPkgName]->bufferCache) {
        auto absPath = FileStore::NormalizePath(file.first);
        if (fileCache.find(absPath) == fileCache.end()) { continue; }
        {
            std::unique_lock<std::recursive_mutex> lock(fileCacheMtx);
            fileCache.erase(absPath);
        }
    }
    packageInstanceCache.erase(dirPath);
#ifdef __linux__
    (void) malloc_trim(0);
#elif __APPLE__
    (void) malloc_zone_pressure_relief(malloc_default_zone(), 0);
#endif
}

void CompilerCangjieProject::FullCompilation()
{
    if (MessageHeaderEndOfLine::GetIsDeveco() && lsp::CjdIndexer::GetInstance() != nullptr) {
        lsp::CjdIndexer::GetInstance()->Build();
    }
    // Construct a dummy instance to load the CJO.
    auto buildCjoTaskId = GenTaskId("buildCjo");
    auto buildCjoTask = [this, buildCjoTaskId]() {
        BuildIndexFromCjo();
        thrdPool->TaskCompleted(buildCjoTaskId);
    };
    thrdPool->AddTask(buildCjoTaskId, {}, buildCjoTask);
    auto sortResult = graph->TopologicalSort(true);

#ifndef TEST_FLAG
    // try to load cache and package status
    for (auto &package : sortResult) {
        if (LoadASTCache(package)) {
            if (!useDB) {
                BuildIndexFromCache(package);
            }
            continue;
        }
    }
#endif

    // submit all package to taskpool, fresh all packagea status
    for (auto& package: sortResult) {
        auto taskId = GenTaskId(package);
        std::unordered_set<uint64_t> dependencies;
        auto allDependencies = graph->FindAllDependencies(package);
        for (auto &iter: allDependencies) {
            dependencies.emplace(GenTaskId(iter));
        }
        auto task = [this, package, taskId]() {
            Trace::Log("start execute task ", package);
            if (CIMap.find(package) == CIMap.end()) {
                thrdPool->TaskCompleted(taskId);
                Trace::Log("package empty, finish execute task ", package);
                return;
            }

            if (cjoManager->GetStatus(package) !=  DataStatus::STALE) {
                cjoManager->UpdateStatus({package}, DataStatus::FRESH);
                thrdPool->TaskCompleted(taskId);
                Trace::Log("finsh execuate task", package);
                return;
            }

            bool changed = CIMap[package]->CompileAfterParse(cjoManager, graph);
            if (changed) {
                Trace::Log("cjo has changed, need to update down stream packages status", package);
                auto downPackages = graph->FindAllDependents(package);
                auto directDownPackages = graph->FindMayDependents(package);
                cjoManager->UpdateStatus(directDownPackages, DataStatus::STALE);
                cjoManager->UpdateStatus(downPackages, DataStatus::WEAKSTALE);
            }
            BuildIndex(CIMap[package], true);
            pLRUCache->SetForFullCompiler(package, CIMap[package]);
            thrdPool->TaskCompleted(taskId);
            Trace::Log("finish execute task ", package);
        };
        thrdPool->AddTask(taskId, dependencies, task);
    }
    thrdPool->WaitUntilAllTasksComplete();
    Trace::Log("All tasks are completed in full compilation");
}

bool CompilerCangjieProject::LoadASTCache(const std::string &package)
{
    if (!cacheManager->IsStale(package, Digest(GetPathFromPkg(package)))) {
        auto I = cacheManager->Load(package);
        CjoData cjoData;
        if (I.has_value()) {
            auto *fileIn = dynamic_cast<lsp::AstFileIn *>(I.value().get());
            cjoData.data = fileIn->data;
            cjoData.status = DataStatus::FRESH;
        } else {
            cjoData.data = {};
            cjoData.status = DataStatus::STALE;
        }
        cjoManager->SetData(package, cjoData);
        return true;
    }

    return false;
}

bool CompilerCangjieProject::Compiler(const std::string &moduleUri,
                                      const nlohmann::json &initializationOptions,
                                      const Environment &environment)
{
    Logger::Instance().LogMessage(MessageType::MSG_INFO, "LD_LIBRARY_PATH is : " + environment.runtimePath);

    // init threadpool
    if (Options::GetInstance().IsOptionSet("test")) {
        thrdPool = std::make_unique<ThrdPool>(1);
    } else {
        thrdPool = std::make_unique<ThrdPool>(PROPER_THREAD_COUNT);
    }

    workspace = FileStore::NormalizePath(URI::Resolve(moduleUri));
    std::string modulesHomeOption;
    if (initializationOptions.contains(MODULES_HOME_OPTION)) {
        modulesHomeOption = initializationOptions.value(MODULES_HOME_OPTION, "");
#ifdef _WIN32
        modulesHomeOption = Cangjie::StringConvertor::NormalizeStringToGBK(modulesHomeOption).value();
#endif
        modulesHomeOption = FileStore::NormalizePath(modulesHomeOption);
    }
    std::string stdLibPathOption;
    if (initializationOptions.contains(STD_LIB_PATH_OPTION)) {
        stdLibPathOption = initializationOptions.value(STD_LIB_PATH_OPTION, "");
#ifdef _WIN32
        stdLibPathOption = Cangjie::StringConvertor::NormalizeStringToGBK(stdLibPathOption).value();
#endif
        stdLibPathOption = FileStore::NormalizePath(stdLibPathOption);
    }
    modulesHome = ::ark::GetModulesHome(modulesHomeOption, environment.cangjieHome);
    stdLibPath = stdLibPathOption;
    cangjiePath = environment.cangjiePath;

    // init moduleManger
    nlohmann::json multiModuleOption = nullptr;
    if (initializationOptions.contains(MULTI_MODULE_OPTION)) {
        multiModuleOption = initializationOptions[MULTI_MODULE_OPTION];
    }
    moduleManager = std::make_unique<ModuleManager>(workspace, multiModuleOption);
    moduleManager->WorkspaceModeParser(moduleUri);
    moduleManager->SetRequireAllPackages();

    // In DevEco, cache path will be pass to create cache manager
    std::string cachePath = workspace;
    if (MessageHeaderEndOfLine::GetIsDeveco() && initializationOptions.contains(CACHE_PATH)) {
        cachePath = initializationOptions.value(CACHE_PATH, "");
    }
    this->cacheManager = std::make_unique<lsp::CacheManager>(cachePath);
#ifndef TEST_FLAG
    cacheManager->InitDir();
#endif

    // get condition compile from initializationOptions
    ark::GetConditionCompile(initializationOptions, passedWhenKeyValue);
    ark::GetModuleConditionCompile(initializationOptions, passedWhenKeyValue, moduleCondition);
    ark::GetSingleConditionCompile(initializationOptions, passedWhenKeyValue, moduleCondition, singlePackageCondition);
    ark::GetConditionCompilePaths(initializationOptions, passedWhenCfgPaths);
    cjcPath = GetCjcPath(environment.runtimePath);
    // get syscap set from condition compile
    SyscapCheck::ParseCondition(GetConditionCompile());
    std::string targetLib = workspace;
    if (initializationOptions.contains(TARGET_LIB)) {
        targetLib = initializationOptions.value(TARGET_LIB, "");
    }
    GetMacroLibPath(targetLib, moduleManager->moduleInfoMap, macroLibs);

    InitPkgInfoAndParse();
    std::string stdCjdPathOption;
    if (MessageHeaderEndOfLine::GetIsDeveco() && initializationOptions.contains(STD_CJD_PATH_OPTION)) {
        stdCjdPathOption = initializationOptions.value(STD_CJD_PATH_OPTION, "");
#ifdef _WIN32
        stdCjdPathOption = Cangjie::StringConvertor::NormalizeStringToGBK(stdCjdPathOption).value();
#endif
        stdCjdPathOption = FileStore::NormalizePath(stdCjdPathOption);
    }
    std::string ohosCjdPathOption;
    if (MessageHeaderEndOfLine::GetIsDeveco() && initializationOptions.contains(OHOS_CJD_PATH_OPTION)) {
        ohosCjdPathOption = initializationOptions.value(OHOS_CJD_PATH_OPTION, "");
#ifdef _WIN32
        ohosCjdPathOption = Cangjie::StringConvertor::NormalizeStringToGBK(ohosCjdPathOption).value();
#endif
        ohosCjdPathOption = FileStore::NormalizePath(ohosCjdPathOption);
    }
    std::string cjdCachePathOption;
    if (MessageHeaderEndOfLine::GetIsDeveco() && initializationOptions.contains(CJD_CACHE_PATH_OPTION)) {
        cjdCachePathOption = initializationOptions.value(CJD_CACHE_PATH_OPTION, "");
#ifdef _WIN32
        cjdCachePathOption = Cangjie::StringConvertor::NormalizeStringToGBK(cjdCachePathOption).value();
#endif
        cjdCachePathOption = FileStore::NormalizePath(cjdCachePathOption);
    }
    if (MessageHeaderEndOfLine::GetIsDeveco()) {
        lsp::CjdIndexer::InitInstance(callback, stdCjdPathOption, ohosCjdPathOption, cjdCachePathOption);
    }
    FullCompilation();
    lsp::CjdIndexer::DeleteInstance();
    lsp::IndexDatabase::ReleaseMemory();
    Logger::Instance().CleanKernelLog(std::this_thread::get_id());
    // init fileCache packageInstance
    for (const auto &item : pLRUCache->GetMpKey()) {
        if (!InitCache(pLRUCache->Get(item), item)) {
            return false;
        }
    }
    auto cycles = graph->FindCycles();
    if (cycles.second) {
        ReportCircularDeps(cycles.first);
    }
    return true;
}

CompilerCangjieProject *CompilerCangjieProject::GetInstance()
{
    return instance;
}

void CompilerCangjieProject::InitInstance(Callbacks *cb, lsp::IndexDatabase *arkIndexDB)
{
    if (instance == nullptr) {
        instance = new(std::nothrow) CompilerCangjieProject(cb, arkIndexDB);
        if (instance == nullptr) {
            Logger::Instance().LogMessage(MessageType::MSG_WARNING, "CompilerCangjieProject::InitInstance fail.");
        }
    }
}

Ptr<Package> CompilerCangjieProject::GetSourcePackagesByPkg(const std::string &fullPkgName)
{
    if (!instance->pLRUCache->Get(fullPkgName) ||
        instance->pLRUCache->Get(fullPkgName)->GetSourcePackages().empty()) {
        return nullptr;
    }
    return instance->pLRUCache->Get(fullPkgName)->GetSourcePackages()[0];
}

std::string CompilerCangjieProject::GetModuleSrcPath(const std::string &modulePath)
{
    if (moduleManager->moduleInfoMap.find(modulePath) == moduleManager->moduleInfoMap.end() ||
        moduleManager->moduleInfoMap[modulePath].srcPath.empty()) {
        return FileStore::NormalizePath(JoinPath(modulePath, SOURCE_CODE_DIR));
    }
    return FileStore::NormalizePath(moduleManager->moduleInfoMap[modulePath].srcPath);
}

void CompilerCangjieProject::UpdateBuffCache(const std::string &file)
{
    auto pkgName = GetFullPkgName(file);
    if (pkgInfoMap.find(pkgName) != pkgInfoMap.end() &&
        !Cangjie::FileUtil::HasExtension(file, CANGJIE_MACRO_FILE_EXTENSION)) {
        {
            std::lock_guard<std::mutex> lock(pkgInfoMap[pkgName]->pkgInfoMutex);
            pkgInfoMap[pkgName]->bufferCache[file] = callback->GetContentsByFile(file);
        }
    }
    if (pkgInfoMapNotInSrc.find(pkgName) != pkgInfoMapNotInSrc.end() &&
        !Cangjie::FileUtil::HasExtension(file, CANGJIE_MACRO_FILE_EXTENSION)) {
        {
            std::lock_guard<std::mutex> lock(pkgInfoMapNotInSrc[pkgName]->pkgInfoMutex);
            pkgInfoMapNotInSrc[pkgName]->bufferCache[file] = callback->GetContentsByFile(file);
        }
    }
    auto downStreamPkgs = graph->FindAllDependents(pkgName);
    cjoManager->UpdateStatus(downStreamPkgs, DataStatus::WEAKSTALE);
}

std::vector<std::vector<std::string>> CompilerCangjieProject::ResolveDependence()
{
    std::vector<std::vector<std::string>> res;
    std::unordered_map<std::string, size_t> dfn, low;
    std::unordered_map<std::string, bool> inSt;
    struct SCCParam param(dfn, low, inSt);
    std::stack<std::string> st;
    size_t index = 0;
    while (param.dfn.size() < CIMap.size()) {
        if (param.dfn.empty()) {
            TarjanForSCC(param, st, index, CIMap.begin()->first, res);
            continue;
        }
        for (auto& i : CIMap) {
            if (!param.dfn[i.first]) {
                TarjanForSCC(param, st, index, i.first, res);
            }
        }
    }
    return res;
}

void CompilerCangjieProject::ReportCircularDeps(const std::vector<std::vector<std::string>> &cycles)
{
    for (const auto &it : cycles) {
        std::string circlePkgName;
        std::set<std::string> sortedPackages(it.begin(), it.end());
        for (const auto &pkg : sortedPackages) {
            circlePkgName += pkg + " ";
        }
        for (const auto &pkg: it) {
            const auto &dirPath = fullPkgNameToPath[pkg];
            std::vector<std::string> files = GetAllFilesUnderCurrentPath(dirPath, CANGJIE_FILE_EXTENSION, false);
            if (files.empty()) {
                continue;
            }
            callback->RemoveDiagOfCurPkg(pkgInfoMap[pkg]->packagePath);
            for (const auto &file: files) {
                const auto &filePath = FileStore::NormalizePath(JoinPath(dirPath, file));
                DiagnosticToken dt;
                dt.category = LSP_ERROR_CODE;
                dt.code = LSP_ERROR_CODE;
                dt.message = "packages " + circlePkgName + "are in circular dependencies.";
                dt.range = {{0, 0, 0}, {0, 0, 1}};
                dt.severity = 1;
                dt.source = "Cangjie";
                callback->UpdateDiagnostic(filePath, dt);
            }
        }
    }
}

void CompilerCangjieProject::TarjanForSCC(SCCParam &sccParam, std::stack<std::string> &st, size_t &index,
                                          const std::string &pkgName, std::vector<std::vector<std::string>> &cycles)
{
    auto& dfn = sccParam.dfn;
    auto& low = sccParam.low;
    auto& inSt = sccParam.inSt;
    dfn[pkgName] = low[pkgName] = ++index;
    st.push(pkgName);
    inSt[pkgName] = true;

    if (!LSPCompilerInstance::dependentPackageMap.count(pkgName)) {
        return;
    }
    for (const auto& outEdge : LSPCompilerInstance::dependentPackageMap[pkgName].downstreamPkgs) {
        if (!dfn[outEdge]) {
            TarjanForSCC(sccParam, st, index, outEdge, cycles);
            low[pkgName] = low[pkgName] < low[outEdge] ? low[pkgName] : low[outEdge];
        } else if (inSt[outEdge]) {
            low[pkgName] = low[pkgName] < low[outEdge] ? low[pkgName] : low[outEdge];
        }
    }
    std::string tmp;
    if (low[pkgName] == dfn[pkgName]) {
        std::vector<std::string> infos;
        do {
            tmp = st.top();
            infos.push_back(tmp);
            st.pop();
            inSt[tmp] = false;
        } while (tmp != pkgName);
        if (infos.size() > 1) {
            cycles.push_back(infos);
        }
    }
}

void CompilerCangjieProject::GetRealPath(std::string &path)
{
    if (!IsRelativePathByImported(path)) {
        return;
    }
    path = Normalize(path);
    std::string dirPath = PathWindowsToLinux(Normalize(GetDirPath(path)));
    if (pkgInfoMap.find(dirPath) != pkgInfoMap.end()) {
        std::string fileName = GetFileName(path);
        path = JoinPath(pkgInfoMap[dirPath]->packagePath, fileName);
    }
}

std::string CompilerCangjieProject::GetFilePathByID(const std::string &curFilePath, unsigned int fileID)
{
    std::string path = GetPathBySource(curFilePath, fileID);
    GetRealPath(path);
    return path;
}

std::string CompilerCangjieProject::GetFilePathByID(const Node &node, unsigned int fileID)
{
    std::string path = GetPathBySource(node, fileID);
    GetRealPath(path);
    return path;
}

void CompilerCangjieProject::CheckPackageNameByAbsName(const File &needCheckedFile, const std::string &fullPackageName)
{
    (void)CheckPackageModifier(needCheckedFile, fullPackageName);

    // check whether the package name is reasonable.
    std::string expectedPkgName = moduleManager->GetExpectedPkgName(needCheckedFile);
    if (needCheckedFile.package == nullptr) {
        if (!expectedPkgName.empty() && expectedPkgName != DEFAULT_PACKAGE_NAME) {
            if (!pkgInfoMap.count(fullPackageName) || pkgInfoMap[fullPackageName]->diag == nullptr) {
                return;
            }
            auto errPos = getPackageNameErrPos(needCheckedFile);
            pkgInfoMap[fullPackageName]->diag->DiagnoseRefactor(DiagKindRefactor::package_name_not_identical_lsp,
                                                                errPos, expectedPkgName);
        }
        return;
    }

    std::string actualPkgName;
    for (auto const &prefix : needCheckedFile.package->prefixPaths) {
        actualPkgName += prefix + CONSTANTS::DOT;
    }
    actualPkgName += needCheckedFile.package->packageName;

    if (actualPkgName != expectedPkgName) {
        auto errPos = getPackageNameErrPos(needCheckedFile);
        pkgInfoMap[fullPackageName]->diag->DiagnoseRefactor(DiagKindRefactor::package_name_not_identical_lsp,
                                                            errPos,
                                                            expectedPkgName);
    }
}

bool CompilerCangjieProject::CheckPackageModifier(const File &needCheckedFile, const std::string &fullPackageName)
{
    if (auto found = fullPackageName.rfind('.'); found != std::string::npos) {
        auto parentPkg = fullPackageName.substr(0, found);
        if (pkgToModMap.count(parentPkg) > 0) {
            auto parentMod = pkgToModMap.at(parentPkg);
            auto curMod = GetPackageSpecMod(needCheckedFile.package.get());
            if (curMod != Modifier::UNDEFINED && parentMod != Modifier::UNDEFINED &&
                static_cast<int>(parentMod) < static_cast<int>(curMod)) {
                DiagnosticToken dt;
                dt.category = LSP_ERROR_CODE;
                dt.code = LSP_ERROR_CODE;
                dt.message = "the access level of child package can't be higher than that of parent package";
                Position begin = {needCheckedFile.package->begin.fileID, needCheckedFile.package->begin.line - 1,
                                  needCheckedFile.package->begin.column - 1};
                Position end = {needCheckedFile.package->end.fileID, needCheckedFile.package->end.line - 1,
                                needCheckedFile.package->end.column - 1};
                dt.range = {begin, end};
                dt.severity = 1;
                dt.source = "Cangjie";
                callback->UpdateDiagnostic(needCheckedFile.filePath, dt);
                return false;
            }
        }
    }
    return true;
}

int CompilerCangjieProject::GetFileID(const std::string &fileName)
{
    std::string fullPkgName = GetFullPkgName(fileName);
    if (pLRUCache->HasCache(fullPkgName)) {
        return pLRUCache->Get(fullPkgName)->GetSourceManager().GetFileID(fileName);
    }
    std::string dirPath = GetDirPath(fileName);
    if (pLRUCache->HasCache(dirPath)) {
        return pLRUCache->Get(dirPath)->GetSourceManager().GetFileID(fileName);
    }
    return 0;
}

int CompilerCangjieProject::GetFileIDForCompete(const std::string &fileName)
{
    ArkAST *ast = GetArkAST(fileName);
    if (ast) {
        return ast->fileID;
    }
    return 0;
}

Callbacks* CompilerCangjieProject::GetCallback()
{
    return callback;
}

bool CompilerCangjieProject::FileHasSemaCache(const std::string &fileName)
{
    std::string fullPkgName = GetFullPkgName(fileName);
    if (pLRUCache->HasCache(fullPkgName)) {
        return true;
    }
    std::string dirPath = GetDirPath(fileName);
    if (pLRUCache->HasCache(dirPath)) {
        return true;
    }
    return false;
}

bool CompilerCangjieProject::CheckNeedCompiler(const std::string &fileName)
{
    std::string fullPkgName = GetFullPkgName(fileName);
    if (pkgInfoMap.find(fullPkgName) != pkgInfoMap.end()) {
        return pkgInfoMap[fullPkgName]->needReCompile;
    }
    std::string dirPath = GetDirPath(fileName);
    if (pkgInfoMapNotInSrc.find(dirPath) != pkgInfoMapNotInSrc.end()) {
        return pkgInfoMapNotInSrc[dirPath]->needReCompile;
    }
    return false;
}

bool CompilerCangjieProject::PkgHasSemaCache(const std::string &pkgName)
{
    return pLRUCache->HasCache(pkgName);
}

std::string CompilerCangjieProject::GetPathBySource(const std::string& fileName, unsigned int id)
{
    std::string fullPkgName = GetFullPkgName(fileName);
    std::string path;
    if (pLRUCache->HasCache(fullPkgName)) {
        path = pLRUCache->Get(fullPkgName)->GetSourceManager().GetSource(id).path;
        GetRealPath(path);
        return path;
    }
    std::string dirPath = GetDirPath(fileName);
    if (pLRUCache->HasCache(dirPath)) {
        path = pLRUCache->Get(dirPath)->GetSourceManager().GetSource(id).path;
        GetRealPath(path);
        return path;
    }
    return "";
}

std::string CompilerCangjieProject::GetPathBySource(const Node &node, unsigned int id)
{
    std::string fullPkgName = GetPkgNameFromNode(&node);
    std::string path;
    if (pLRUCache->HasCache(fullPkgName)) {
        path = pLRUCache->Get(fullPkgName)->GetSourceManager().GetSource(id).path;
        GetRealPath(path);
        return path;
    }
    auto fileNode = dynamic_cast<const File*>(&node);
    if (fileNode == nullptr && node.curFile == nullptr) {
        return "";
    }
    path = fileNode == nullptr ? node.curFile->filePath : fileNode->filePath;
    std::string dirPath = GetDirPath(path);
    if (pLRUCache->HasCache(dirPath)) {
        path = pLRUCache->Get(dirPath)->GetSourceManager().GetSource(id).path;
        GetRealPath(path);
        return path;
    }
    return "";
}

void CompilerCangjieProject::ClearParseCache()
{
    CIForParse.reset();
    packageInstanceCacheForParse.reset();
    fileCacheForParse.clear();
}

std::vector<std::string> CompilerCangjieProject::GetMacroLibs() const
{
    return macroLibs;
}

std::string CompilerCangjieProject::GetCjc() const
{
    return cjcPath;
}

std::unordered_map<std::string, std::string> CompilerCangjieProject::GetConditionCompile(
    const std::string &packageName, const std::string &moduleName) const
{
    if (singlePackageCondition.find(packageName) != singlePackageCondition.end()) {
        return singlePackageCondition.at(packageName);
    }
    if (moduleCondition.find(moduleName) != moduleCondition.end()) {
        return moduleCondition.at(moduleName);
    }
    return passedWhenKeyValue;
}

std::unordered_map<std::string, std::string> CompilerCangjieProject::GetConditionCompile() const
{
    return passedWhenKeyValue;
}

std::vector<std::string> CompilerCangjieProject::GetConditionCompilePaths() const
{
    return passedWhenCfgPaths;
}

void CompilerCangjieProject::GetDiagCurEditFile(const std::string &file)
{
    std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(file);
    int64_t version = callback->GetVersionByFile(file);
    callback->ReadyForDiagnostics(file, version, diagnostics);
}

void CompilerCangjieProject::StoreAllPackagesCache()
{
    for (auto& [fullPkgName, _]: fullPkgNameToPath) {
        StorePackageCache(fullPkgName);
    }
}

void CompilerCangjieProject::StorePackageCache(const std::string& pkgName)
{
    if (!useDB) {
        std::string sourceCodePath;
        if (fullPkgNameToPath.find(pkgName) != fullPkgNameToPath.end()) {
            sourceCodePath = this->fullPkgNameToPath.find(pkgName)->second;
        } else {
            sourceCodePath = pkgName;
        }
        auto shardIdentifier = Digest(sourceCodePath);
        auto shard = lsp::IndexFileOut();
        shard.symbols = &memIndex->pkgSymsMap[pkgName];
        shard.refs = &memIndex->pkgRefsMap[pkgName];
        shard.relations = &memIndex->pkgRelationsMap[pkgName];
        shard.extends = &memIndex->pkgExtendsMap[pkgName];
        shard.crossSymbos = &memIndex->pkgCrossSymsMap[pkgName];
        cacheManager->StoreIndexShard(pkgName, shardIdentifier, shard);
    }
    cacheManager->Store(
        pkgName, Digest(GetPathFromPkg(pkgName)), *cjoManager->GetData(pkgName));
}

void CompilerCangjieProject::BuildIndex(const std::unique_ptr<LSPCompilerInstance> &ci, bool isFullCompilation)
{
    auto packages = ci->GetSourcePackages();
    if (!packages[0] || !ci->typeManager) {
        return;
    }
    std::string curPkgName = ci->pkgNameForPath;
    std::map<std::string, std::unique_ptr<ArkAST>> astMap;
    std::map<int, std::vector<std::string>> fileMap;
    for (auto pkg : ci->GetSourcePackages()) {
        if (pkg->files.empty()) {
            continue;
        }
        // if pkg->files[0]->filePath is a dir file we need dirPath is pkgForPath
        std::string dirPath = Normalize(GetDirPath(pkg->files[0]->filePath));
        if (GetFileExtension(pkg->files[0]->filePath) != "cj") {
            dirPath = Normalize(pkg->files[0]->filePath);
        }

        for (auto &file : pkg->files) {
            if (!file || !file->curPackage) {
                continue;
            }
            auto filePath = file->filePath;
            // filePath maybe a dir not a file
            if (GetFileExtension(filePath) != "cj") { continue; }
            LowFileName(filePath);
            std::string contents;

            if (pkgInfoMap.find(curPkgName) == pkgInfoMap.end()) {
                continue;
            }
            {
                std::lock_guard<std::mutex> lock(pkgInfoMap[curPkgName]->pkgInfoMutex);
                contents = pkgInfoMap[curPkgName]->bufferCache[filePath];
            }

            std::pair<std::string, std::string> paths = {filePath, contents};
            auto arkAST = std::make_unique<ArkAST>(paths, file.get(), ci->diag, packageInstanceCache[dirPath].get(),
                                                   &ci->GetSourceManager());
            std::string absName = FileStore::NormalizePath(filePath);
            std::string moduleName = SplitFullPackage(curPkgName).first;
            auto id = GetFileIdForDB(absName);
            std::vector<std::string> fileInfo;
            fileInfo.emplace_back(absName);
            fileInfo.emplace_back(curPkgName);
            fileInfo.emplace_back(moduleName);
            auto digest = Digest(absName);
            fileInfo.emplace_back(digest);
            fileMap.insert(std::make_pair(id, fileInfo));
            int fileId = ci->GetSourceManager().GetFileID(absName);
            if (fileId >= 0) {
                arkAST->fileID = static_cast<unsigned int>(fileId);
            }
            {
                std::unique_lock<std::recursive_mutex> lock(fileCacheMtx);
                astMap[absName] = std::move(arkAST);
            }
        }
    }

    // Need to rebuild index
    lsp::SymbolCollector sc = lsp::SymbolCollector(*ci->typeManager, ci->importManager, false);
    sc.SetArkAstMap(std::move(astMap));
    sc.Build(*packages[0]);
    if (useDB) {
        auto shard = lsp::IndexFileOut();
        shard.symbols = sc.GetSymbolMap();
        shard.refs = sc.GetReferenceMap();
        shard.relations = sc.GetRelations();
        shard.extends = sc.GetSymbolExtendMap();
        shard.crossSymbos = sc.GetCrossSymbolMap();
        backgroundIndexDb->UpdateFile(fileMap);
        backgroundIndexDb->Update(curPkgName, shard);
    } else {
        // Merge index to memory
        std::unique_lock<std::mutex> indexLock(indexMtx);
        (void) memIndex->pkgSymsMap.insert_or_assign(curPkgName, *sc.GetSymbolMap());
        (void) memIndex->pkgRefsMap.insert_or_assign(curPkgName, *sc.GetReferenceMap());
        (void) memIndex->pkgRelationsMap.insert_or_assign(curPkgName, *sc.GetRelations());
        (void) memIndex->pkgExtendsMap.insert_or_assign(curPkgName, *sc.GetSymbolExtendMap());
        (void) memIndex->pkgCrossSymsMap.insert_or_assign(curPkgName, *sc.GetCrossSymbolMap());
        indexLock.unlock();
    }

#ifndef TEST_FLAG
    // Store the indexs and astdata
    if (isFullCompilation) {
        auto needStoreCache = MessageHeaderEndOfLine::GetIsDeveco()
                                  ? ci->diag.GetErrorCount() == 0 : ci->macroExpandSuccess;
        if (needStoreCache) {
            StorePackageCache(curPkgName);
        }
        Trace::Log(curPkgName, "error count: ", ci->diag.GetErrorCount());
    }
#endif
}

void CompilerCangjieProject::UpdateOnDisk(const std::string &path)
{
    auto found = pathToFullPkgName.find(FileUtil::GetDirPath(path));
    if (found == pathToFullPkgName.end()) {
        return;
    }
    std::string pkgName = found->second;
    DataStatus status = cjoManager->GetStatus(pkgName);
    cacheManager->Store(pkgName, Digest(GetPathFromPkg(pkgName)), LSPCompilerInstance::astDataMap[pkgName].first);
}

Position CompilerCangjieProject::getPackageNameErrPos(const File &file) const
{
    // packagePos not exist return file.begin
    if (!file.package || file.package->packagePos.IsZero()) {
        return file.begin;
    }
    // packageNamePos not exist return packagePos
    return file.package->packageName.Begin().IsZero() ? file.package->packagePos : file.package->packageName.Begin();
}

std::string CompilerCangjieProject::Denoising(std::string candidate)
{
    return pkgInfoMap.count(candidate) ? candidate :"";
}

ark::Modifier CompilerCangjieProject::GetPackageSpecMod(Node *node)
{
    if (!node) {
        return ark::Modifier::UNDEFINED;
    }
    if (node->TestAttr(Attribute::PUBLIC)) {
        return ark::Modifier::PUBLIC;
    } else if (node->TestAttr(Attribute::PROTECTED)) {
        return ark::Modifier::PROTECTED;
    } else if (node->TestAttr(Attribute::INTERNAL)) {
        return ark::Modifier::INTERNAL;
    } else if (node->TestAttr(Attribute::PRIVATE)) {
        return ark::Modifier::PRIVATE;
    }
    return ark::Modifier::UNDEFINED;
}

bool CompilerCangjieProject::IsVisibleForPackage(const std::string &curPkgName, const std::string &importPkgName)
{
    auto importPkgPath = GetPathFromPkg(importPkgName);
    if (!FileUtil::FileExist(importPkgPath)) {
        return false;
    }
    auto found = pkgToModMap.find(importPkgName);
    if (found == pkgToModMap.end()) {
        return false;
    }
    auto importModifer = pkgToModMap[importPkgName];
    auto relation = ::GetPkgRelation(curPkgName, importPkgName);
    return importModifer == Modifier::PUBLIC ||
           (importModifer == Modifier::PROTECTED && relation != PkgRelation::NONE) ||
           (importModifer == Modifier::INTERNAL && relation == PkgRelation::CHILD);
}

bool CompilerCangjieProject::IsCurModuleCjoDep(const std::string &curModule, const std::string &fullPkgName)
{
    for (auto &allCjoDeps : LSPCompilerInstance::usrCjoFileCacheMap) {
        if (curModule != allCjoDeps.first) {
            continue;
        }
        for (auto &cjoDep : allCjoDeps.second) {
            if (cjoDep.first == fullPkgName) {
                return true;
            }
        }
    }
    return false;
}

void CompilerCangjieProject::BuildIndexFromCjo()
{
    auto pi = std::make_unique<PkgInfo>("", "", "", callback);
    auto ci =
        std::make_unique<LSPCompilerInstance>(callback, *pi->compilerInvocation, *pi->diag, "dummy", moduleManager);
    ci->IndexCjoToManager(cjoManager, graph);
    std::map<int, std::vector<std::string>> fileMap;
    for (const auto &cjoPath : ci->cjoPathSet) {
        std::string cjoName = FileUtil::GetFileNameWithoutExtension(cjoPath);
        auto cjoPkg = ci->importManager.LoadPackageFromCjo(cjoName, cjoPath);
        if (!cjoPkg) {
            continue;
        }
        auto cjoPkgName = cjoPkg->fullPackageName;
        bool toUpdateDB = true;
        std::string digest;
        if (useDB) {
            auto cjoID = GetFileIdForDB(cjoPath);
            digest = DigestForCjo(cjoPath);
            std::string old_digest = backgroundIndexDb->GetFileDigest(cjoID);
            if (digest != old_digest) {
                std::vector<std::string> cjoInfo;
                std::string emptyStr;
                cjoInfo.emplace_back(cjoPkgName);
                cjoInfo.emplace_back(emptyStr);
                cjoInfo.emplace_back(emptyStr);
                cjoInfo.emplace_back(digest);
                fileMap.insert(std::make_pair(cjoID, cjoInfo));
            } else {
                toUpdateDB = false;
          }
        }
        lsp::SymbolCollector sc = lsp::SymbolCollector(*ci->typeManager, ci->importManager, true);
        if (!useDB || (useDB && toUpdateDB)) {
            Trace::Log("build for cjo:", cjoPkgName);
            sc.Build(*cjoPkg);
        }
        if (useDB && toUpdateDB) {
            for (auto sym: *sc.GetSymbolMap()) {
                auto id = GetFileIdForDB(sym.location.fileUri);
                if (fileMap.find(id) == fileMap.end()) {
                    std::vector<std::string> fileInfo;
                    fileInfo.emplace_back(sym.location.fileUri);
                    fileInfo.emplace_back(cjoPkgName);
                    fileInfo.emplace_back(sym.curModule);
                    fileInfo.emplace_back(digest);
                    fileMap.insert(std::make_pair(id, fileInfo));
                }
            }
            (void) memIndex->pkgSymsMap.insert_or_assign(cjoPkgName, *sc.GetSymbolMap());
            (void) memIndex->pkgRefsMap.insert_or_assign(cjoPkgName, *sc.GetReferenceMap());
            (void) memIndex->pkgRelationsMap.insert_or_assign(cjoPkgName, *sc.GetRelations());
            (void) memIndex->pkgExtendsMap.insert_or_assign(cjoPkgName, *sc.GetSymbolExtendMap());
            (void) memIndex->pkgCrossSymsMap.insert_or_assign(cjoPkgName, *sc.GetCrossSymbolMap());
        } else if (!useDB) {
            // Merge index to memory
            (void) memIndex->pkgSymsMap.insert_or_assign(cjoPkgName, *sc.GetSymbolMap());
            (void) memIndex->pkgRefsMap.insert_or_assign(cjoPkgName, *sc.GetReferenceMap());
            (void) memIndex->pkgRelationsMap.insert_or_assign(cjoPkgName, *sc.GetRelations());
            (void) memIndex->pkgExtendsMap.insert_or_assign(cjoPkgName, *sc.GetSymbolExtendMap());
            (void) memIndex->pkgCrossSymsMap.insert_or_assign(cjoPkgName, *sc.GetCrossSymbolMap());
        }
    }
    if (useDB) {
        Trace::Log("UpdateAll Start");
        backgroundIndexDb->UpdateAll(fileMap, std::move(memIndex));
        Trace::Log("UpdateAll End");
    }
}

void CompilerCangjieProject::BuildIndexFromCache(const std::string &package) {
        std::string sourceCodePath;
        if (fullPkgNameToPath.find(package) != fullPkgNameToPath.end()) {
            sourceCodePath = this->fullPkgNameToPath.find(package)->second;
        } else {
            sourceCodePath = package;
        }
        std::string shardIdentifier = Digest(sourceCodePath);
        auto indexCache = cacheManager->LoadIndexShard(package, shardIdentifier);
        if (!indexCache.has_value()) {
            return;
        }
        {
            std::unique_lock<std::mutex> indexLock(mtx);
            (void) memIndex->pkgSymsMap.insert_or_assign(package, indexCache->get()->symbols);
            (void) memIndex->pkgRefsMap.insert_or_assign(package, indexCache->get()->refs);
            (void) memIndex->pkgRelationsMap.insert_or_assign(package, indexCache->get()->relations);
            (void) memIndex->pkgExtendsMap.insert_or_assign(package, indexCache->get()->extends);
            (void) memIndex->pkgCrossSymsMap.insert_or_assign(package, indexCache->get()->crossSymbos);
        }
}

std::unordered_set<std::string> CompilerCangjieProject::GetOneModuleDeps(const std::string &curModule)
{
    std::unordered_set<std::string> res;
    auto found = moduleManager->requireAllPackages.find(curModule);
    if (found != moduleManager->requireAllPackages.end()) {
        return found->second;
    }
    return res;
}

std::unordered_set<std::string> CompilerCangjieProject::GetOneModuleDirectDeps(const std::string &curModule)
{
    std::unordered_set<std::string> res;
    auto found = moduleManager->requirePackages.find(curModule);
    if (found != moduleManager->requirePackages.end()) {
        return found->second;
    }
    return res;
}

} // namespace ark
