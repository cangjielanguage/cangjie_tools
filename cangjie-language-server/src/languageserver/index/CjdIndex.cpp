// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <fstream>
#include "CjdIndex.h"

namespace Cangjie {

std::string DCompilerInstance::Denoising(std::string candidate)
{
    return ark::lsp::CjdIndexer::GetInstance()->GetPkgMap().count(candidate) ? candidate : "";
}

void DCompilerInstance::ImportCjoToManager(const std::unique_ptr<ark::CjoManager> &cjoManager,
                                           const std::unique_ptr<ark::DependencyGraph> &graph)
{
    // Import stdlib cjo, priority is low.
    for (const auto &cjoCache: cjoFileCacheMap) {
        importManager.SetPackageCjoCache(cjoCache.first, cjoCache.second);
    }

    auto allDependencies = graph->FindAllDependencies(pkgNameForPath);
    for (auto &package: allDependencies) {
        for (auto &item: usrCjoFileCacheMap) {
            if (item.second.count(package)) {
                importManager.SetPackageCjoCache(package, item.second[package]);
            }
        }
    }
}
} // namespace Cangjie

namespace ark {
namespace  lsp {
CjdIndexer *CjdIndexer::instance = nullptr;

void CjdIndexer::InitInstance(Callbacks *cb, const std::string& stdCjdPathOption,
                              const std::string& ohosCjdPathOption, const std::string& cjdCachePathOption)
{
    if (instance == nullptr) {
        instance = new(std::nothrow) CjdIndexer(cb, stdCjdPathOption,
                                                ohosCjdPathOption, cjdCachePathOption);
        if (instance == nullptr) {
            Logger::Instance().LogMessage(MessageType::MSG_WARNING, "CjdIndexer::InitInstance fail.");
        }
    }
}

CjdIndexer *CjdIndexer::GetInstance()
{
    return instance;
}

void CjdIndexer::LoadAllCJDResource()
{
    // 1. load all cj.d file, construct package info
    Trace::Log("LoadAllCJDResource start");
    std::vector<std::string> cjdPaths;
    cjdPaths.emplace_back(stdCjdPath);
    cjdPaths.emplace_back(ohosCjdPath);
    std::map<int, std::vector<std::string>> fileMap;
    for (auto &cjdPath: cjdPaths) {
        if (!enablePackaged) {
            for (auto &modulePath: FileUtil::GetAllDirsUnderCurrentPath(cjdPath)) {
                // just handle the level-1 subdirectory
                if (IsFirstSubDir(cjdPath, modulePath)) {
                    ReadCJDSource(modulePath, modulePath, fileMap);
                }
            }
        } else {
            for (const auto &packagedCjdFile :
                FileUtil::GetAllFilesUnderCurrentPath(cjdPath, "d")) {
                ReadPackagedCjdResource(cjdPath, packagedCjdFile, fileMap);
            }
        }
    }
    if (CompilerCangjieProject::GetUseDB()) {
        CompilerCangjieProject::GetInstance()->GetBgIndexDB()->UpdateFile(fileMap);
    }
    Trace::Log("LoadAllCJDResource end");
}

void CjdIndexer::ParsePackageDependencies()
{
    // 2. parse package dependencies
    Trace::Log("ParsePackageDependencies start");
    for (auto &item: pkgMap) {
        auto ci = std::make_unique<DCompilerInstance>(
                callback, *item.second->compilerInvocation, *item.second->diag, item.first);
        ci->cangjieHome = CompilerCangjieProject::GetInstance()->GetModulesHome();
        ci->loadSrcFilesFromCache = true;
        ci->bufferCache = item.second->bufferCache;
        ci->PreCompileProcess();
        std::string fullPackageName = item.first;
        ci->UpdateDepGraph(graph, item.first);
        ciMap[fullPackageName] = std::move(ci);
    }
    Trace::Log("ParsePackageDependencies end");
}

void CjdIndexer::BuildCJDIndex()
{
    // 3. compiler all packages, build cj.d index
    Trace::Log("BuildCJDIndex start");
    auto sortResult = graph->TopologicalSort(true);
    for (auto &package: sortResult) {
        auto taskId = GenTaskId(package);
        std::unordered_set<uint64_t> dependencies;
        auto allDependencies = graph->FindAllDependencies(package);
        auto task = [this, package, taskId]() {
            Trace::Log("start execute task ", package);
            (void) ciMap[package]->ImportCjoToManager(cjoManager, graph);
            (void) ciMap[package]->ImportPackage();
            (void) ciMap[package]->MacroExpand();
            (void) ciMap[package]->Sema();
            (void) ExecuteCompilerApi("DeleteASTLoaders", &ImportManager::DeleteASTLoaders,
                                      ciMap[package]->importManager);
            auto packages = ciMap[package]->GetSourcePackages();
            std::vector<uint8_t> data;
            (void) ciMap[package]->ExportAST(false, data, *packages[0]);
            cjoManager->SetData(package, {data, DataStatus::FRESH});
            lsp::SymbolCollector sc = lsp::SymbolCollector(*ciMap[package]->typeManager,
                                                           ciMap[package]->importManager, false);
            sc.Build(*packages[0]);
            pkgSymsMap.insert_or_assign(package, *sc.GetSymbolMap());
            auto shardIdentifier = "cjd";
            auto shard = lsp::IndexFileOut();
            shard.symbols = sc.GetSymbolMap();
            shard.refs = sc.GetReferenceMap();
            shard.relations = sc.GetRelations();
            shard.extends = sc.GetSymbolExtendMap();
            shard.crossSymbos = sc.GetCrossSymbolMap();
            cacheManager->StoreIndexShard(package, shardIdentifier, shard);
            thrdPool->TaskCompleted(taskId);
            Trace::Log("finish execute task ", package);
        };
        thrdPool->AddTask(taskId, dependencies, task);
    }
    thrdPool->WaitUntilAllTasksComplete();
    Trace::Log("BuildCJDIndex end");
}

SymbolLocation CjdIndexer::GetSymbolDeclaration(SymbolID id, const std::string& fullPkgName)
{
    SymbolLocation loc;
    if (auto found = pkgSymsMap.find(fullPkgName); found != pkgSymsMap.end()) {
        for (auto &sym: found->second) {
            if (sym.id == id) {
                loc = sym.location;
                break;
            }
        }
    }
    return loc;
}

CommentGroups CjdIndexer::GetSymbolComments(SymbolID id, const std::string& fullPkgName)
{
    CommentGroups comments;
    if (auto found = pkgSymsMap.find(fullPkgName); found != pkgSymsMap.end()) {
        for (auto &sym: found->second) {
            if (sym.id == id) {
                comments = sym.comments;
                break;
            }
        }
    }
    return comments;
}

void CjdIndexer::ReadCJDSource(const std::string &rootPath, const std::string &modulePath,
                               std::map<int, std::vector<std::string>> &fileMap, const std::string &parentPkg)
{
    std::string dirName = FileUtil::GetDirName(rootPath);
    std::string currentPkg = parentPkg.empty() ? dirName : parentPkg + "." + dirName;
    pkgMap[currentPkg] =
            std::make_unique<DPkgInfo>(rootPath, modulePath,
                                       FileUtil::GetDirName(modulePath), callback);
    auto allFiles = GetAllFilesUnderCurrentPath(rootPath, "d");
    for (auto &file: allFiles) {
        auto filePath = NormalizePath(JoinPath(rootPath, file));
        LowFileName(filePath);
        pkgMap[currentPkg]->bufferCache.emplace(filePath, GetFileContents(filePath));

        auto id = GetFileIdForDB(filePath);
        std::vector<std::string> fileInfo;
        fileInfo.emplace_back(filePath);
        fileInfo.emplace_back("");
        fileInfo.emplace_back("");
        auto digest = Digest(filePath);
        fileInfo.emplace_back(digest);
        fileMap.insert(std::make_pair(id, fileInfo));
    }
    for (auto &childPath: FileUtil::GetAllDirsUnderCurrentPath(rootPath)) {
        if (FileUtil::IsDir(childPath)) {
            ReadCJDSource(childPath, modulePath, fileMap, currentPkg);
        }
    }
}

void CjdIndexer::ReadPackagedCjdResource(const std::string& rootPath, const std::string& filePath,
    std::map<int, std::vector<std::string>> &fileMap)
{
    auto pkgName = FileUtil::GetFileBase(FileUtil::GetFileBase(filePath));
    auto normalizedPath = NormalizePath(JoinPath(rootPath, filePath));
    LowFileName(normalizedPath);
    auto ModuleName = pkgName.substr(0, pkgName.find_first_of('.'));
    pkgMap[pkgName] = std::make_unique<DPkgInfo>(rootPath, rootPath, ModuleName, callback);
    pkgMap[pkgName]->bufferCache.emplace(normalizedPath, GetFileContents(normalizedPath));

    auto id = GetFileIdForDB(normalizedPath);
    std::vector<std::string> fileInfo;
    fileInfo.emplace_back(normalizedPath);
    fileInfo.emplace_back("");
    fileInfo.emplace_back("");
    auto digest = Digest(normalizedPath);
    fileInfo.emplace_back(digest);
    fileMap.insert(std::make_pair(id, fileInfo));
}

void CjdIndexer::BuildIndexFromCache()
{
    Trace::Log("BuildIndexFromCache Start");
    std::string cjdIndexDir = JoinPath(JoinPath(cjdCachePath, ".cache"), "index");
    std::map<int, std::vector<std::string>> fileMap;
    for (auto& idxFile:
            FileUtil::GetAllFilesUnderCurrentPath(cjdIndexDir, "idx")) {
        auto package = FileUtil::GetFileBase(FileUtil::GetFileBase(idxFile));
        std::string shardIdentifier = "cjd";
        auto indexCache = cacheManager->LoadIndexShard(package, shardIdentifier);
        if (!indexCache.has_value()) {
            Trace::Log("BuildIndexFromCache failed", package);
            return;
        }
        {
            std::unique_lock<std::mutex> indexLock(mtx);
            (void) pkgSymsMap.insert_or_assign(package, indexCache->get()->symbols);
        }

        // update db file table
        if (!CompilerCangjieProject::GetUseDB() || indexCache->get()->symbols.empty()) {
            continue;
        }
        const auto& sym = indexCache->get()->symbols[0];
        std::string absName = FileStore::NormalizePath(sym.location.fileUri);
        std::string curDigest = Digest(absName);
        auto id = GetFileIdForDB(absName);
        std::string oldDigest = CompilerCangjieProject::GetInstance()->GetBgIndexDB()->GetFileDigest(id);
        if (curDigest == oldDigest) {
            continue;
        }
        std::vector<std::string> fileInfo;
        fileInfo.emplace_back(absName);
        fileInfo.emplace_back("");
        fileInfo.emplace_back("");
        fileInfo.emplace_back(curDigest);
        fileMap.insert(std::make_pair(id, fileInfo));

        // check is contain macrocall file
        std::string macroCallFile = FileStore::NormalizePath(JoinPath(sym.location.fileUri, "macrocall"));
        if (!FileExist(macroCallFile)) {
            continue;
        }
        std::string curMacroCallDigest = Digest(macroCallFile);
        auto macroCallId = GetFileIdForDB(absName);
        std::string oldMacroCallDigest =
            CompilerCangjieProject::GetInstance()->GetBgIndexDB()->GetFileDigest(macroCallId);
        if (curMacroCallDigest == oldMacroCallDigest) {
            continue;
        }
        std::vector<std::string> macroCallFileInfo;
        macroCallFileInfo.emplace_back(macroCallFile);
        macroCallFileInfo.emplace_back("");
        macroCallFileInfo.emplace_back("");
        macroCallFileInfo.emplace_back(curMacroCallDigest);
        fileMap.insert(std::make_pair(macroCallId, macroCallFileInfo));
    }
    if (CompilerCangjieProject::GetUseDB()) {
        CompilerCangjieProject::GetInstance()->GetBgIndexDB()->UpdateFile(fileMap);
    }
    Trace::Log("BuildIndexFromCache End");
}

void CjdIndexer::Build()
{
    if (CheckCjdCache()) {
        BuildIndexFromCache();
        return;
    }
    isIndexing = true;
    LoadAllCJDResource();
    ParsePackageDependencies();
    BuildCJDIndex();
    GenerateValidFile();
    isIndexing = false;
}

bool CjdIndexer::CheckCjdCache()
{
    const std::string validFile = JoinPath(cjdCachePath, "valid.txt");
    std::string reason;
    if (FileExist(validFile) && ReadFileContent(validFile, reason).value_or("") == GetValidCode()) {
        return true;
    }
    return false;
}

std::string CjdIndexer::GetValidCode()
{
    std::string contents;
    std::string reason;
    for (auto& file: FileUtil::GetAllFilesUnderCurrentPath(cjdCachePath, "idx")) {
        contents += file + FileUtil::ReadFileContent(file, reason).value_or("");
    }
    return std::to_string(std::hash<std::string>{}(contents));
}

void CjdIndexer::GenerateValidFile()
{
    Trace::Log("Generate Cjd Index Valid Files Start");
    std::ofstream validFile;
    validFile.open(Normalize(JoinPath(cjdCachePath, "valid.txt")));
    if (!validFile.is_open()) {
        Trace::Log("Create cjd index files valid file failed");
    }
    validFile << GetValidCode();
    if (validFile.fail()) {
        Trace::Log("Write cjd index files valid file failed");
    }
    validFile.close();
    Trace::Log("Generate Cjd Index Valid Files End");
}
} // namespace lsp
} // namespace ark
