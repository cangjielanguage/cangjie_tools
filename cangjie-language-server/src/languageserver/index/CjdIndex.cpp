// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <fstream>
#include <sstream>
#include "CjdIndex.h"
#include "../common/FileStore.h"

namespace Cangjie {
// LCOV_EXCL_START
std::string DCompilerInstance::Denoising(std::string candidate)
{
    return ark::lsp::CjdIndexer::GetInstance()->GetPkgMap().count(candidate) ? candidate : "";
}

void DCompilerInstance::ImportCjoToManager(const std::unique_ptr<ark::CjoManager> &cjoManager,
                                           const std::unique_ptr<ark::DependencyGraph> &graph)
{
    (void)cjoManager;
    // Import stdlib cjo, priority is low.
    for (const auto &cjoCache: cjoFileCacheMap) {
        importManager->SetPackageCjoCache(cjoCache.first, cjoCache.second);
    }

    auto allDependencies = graph->FindAllDependencies(pkgNameForPath);
    for (auto &package: allDependencies) {
        for (auto &item: usrCjoFileCacheMap) {
            auto it = item.second.find(package);
            if (it != item.second.end()) {
                importManager->SetPackageCjoCache(package, *it->second);
            }
        }
    }
}
} // namespace Cangjie
// LCOV_EXCL_STOP
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
        auto ci = std::make_unique<DCompilerInstance>(callback, *item.second->compilerInvocation,
                CompilerCangjieProject::GetInstance()->GetDiagnosticEngine(), item.first);
        ci->cangjieHome = CompilerCangjieProject::GetInstance()->GetModulesHome();
        ci->loadSrcFilesFromCache = true;
        ci->SetBufferCache(item.second->bufferCache);
        ci->PreCompileProcess();
        std::string fullPackageName = item.first;
        ci->UpdateDepGraph(graph, item.first);
        ciMap[fullPackageName] = std::move(ci);
        // LCOV_EXCL_STOP
    }
    Trace::Log("ParsePackageDependencies end");
}

void CjdIndexer::BuildCJDIndex()
{
    // 3. compiler all packages, build cj.d index
    Trace::Log("BuildCJDIndex start");
    auto sortResult = graph->TopologicalSort(true);
    for (auto &package: sortResult) {
        // LCOV_EXCL_START
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
                                      *ciMap[package]->importManager);
            auto packages = ciMap[package]->GetSourcePackages();
            std::vector<uint8_t> data;
            (void) ciMap[package]->ExportAST(false, data, *packages[0]);
            cjoManager->SetData(package, {data, DataStatus::FRESH});
            lsp::SymbolCollector sc = lsp::SymbolCollector(*ciMap[package]->typeManager,
                                                           *ciMap[package]->importManager, false);
            sc.Build(*packages[0]);
            pkgSymsMap.insert_or_assign(package, *sc.GetSymbolMap());
            auto shardIdentifier = "cjd";
            auto shard = lsp::IndexFileOut();
            shard.symbols = sc.GetSymbolMap();
            shard.refs = sc.GetReferenceMap();
            shard.relations = sc.GetRelations();
            shard.extends = sc.GetSymbolExtendMap();
            shard.crossSymbols = sc.GetCrossSymbolMap();
            shard.reExportSymbols = sc.GetReExportSymbolMap();
            cacheManager->StoreIndexShard(package, shardIdentifier, shard);
            thrdPool->TaskCompleted(taskId);
            Trace::Log("finish execute task ", package);
        };
        thrdPool->AddTask(taskId, dependencies, task);
        // LCOV_EXCL_STOP
    }
    thrdPool->WaitUntilAllTasksComplete();
    Trace::Log("BuildCJDIndex end");
}

const Symbol *CjdIndexer::GetSymbol(SymbolID id, const std::string& fullPkgName)
{
    if (auto found = pkgSymsMap.find(fullPkgName); found != pkgSymsMap.end()) {
        for (const auto &sym : found->second) {
            if (sym.id == id) {
                return &sym;
            }
        }
    }
    return nullptr;
}

void CjdIndexer::BuildSourceLineIndex()
{
    for (const auto &[pkgName, pkg] : pkgMap) {
        auto &files = pkgLineIndex[pkgName];
        for (const auto &[path, content] : pkg->bufferCache) {
            std::vector<size_t> offsets;
            offsets.push_back(0);
            for (size_t i = 0; i < content.size(); ++i) {
                if (content[i] == '\n') {
                    offsets.push_back(i + 1);
                }
            }
            files.emplace(path, std::move(offsets));
        }
    }
}

std::string CjdIndexer::ExtractSignatureFromSource(const std::string& identifier, const Position& begin,
                                                   const Position& end, const std::string& fullPkgName,
                                                   const std::string& filePath)
{
    auto pkg = pkgMap.find(fullPkgName);
    // Cangjie Position: line and column start at 1, column is a byte offset within the line.
    if (pkg == pkgMap.end() || begin.line <= 0 || end.line < begin.line) { return {}; }
    auto lineIdx = pkgLineIndex.find(fullPkgName);
    if (lineIdx == pkgLineIndex.end()) { return {}; }

    auto byteOffset = [](const std::string_view &line, int column) -> size_t {
        return column <= 1 ? 0 : std::min(static_cast<size_t>(column - 1), line.size());
    };
    // Slice lines [begin.line, end.line] using the precomputed per-line offsets (O(decl lines)).
    auto sliceFrom = [&begin, &end, &byteOffset](const std::string &content,
                                                 const std::vector<size_t> &offsets) -> std::string {
        if (static_cast<size_t>(end.line) > offsets.size()) { return {}; }
        std::string signature;
        for (int lineNo = begin.line; lineNo <= end.line; ++lineNo) {
            const size_t start = offsets[static_cast<size_t>(lineNo - 1)];
            size_t lineEnd = static_cast<size_t>(lineNo) < offsets.size()
                ? offsets[static_cast<size_t>(lineNo)] - 1 : content.size(); // exclude the trailing '\n'
            while (lineEnd > start && content[lineEnd - 1] == '\r') { --lineEnd; } // Windows CRLF
            const std::string_view line(content.data() + start, lineEnd - start);
            const size_t first = lineNo == begin.line ? byteOffset(line, begin.column) : 0;
            const size_t last  = lineNo == end.line   ? byteOffset(line, end.column)   : line.size();
            if (first > last) { return {}; }
            signature += line.substr(first, last - first);
            if (lineNo != end.line) { signature += ' '; }
        }
        return signature;
    };

    // Fast path: locate the source buffer by path hint, avoiding a scan across all files.
    if (!filePath.empty()) {
        for (const auto &[path, content] : pkg->second->bufferCache) {
            if (path != filePath && path.find(filePath) == std::string::npos) {
                continue;
            }
            auto found = lineIdx->second.find(path);
            if (found == lineIdx->second.end()) { continue; }
            auto signature = sliceFrom(content, found->second);
            if (!signature.empty() && signature.find(identifier) != std::string::npos) {
                return signature;
            }
        }
    }

    // A package may span several .d files; positions are per-file, so pick the file whose
    // sliced text actually contains the identifier.
    for (const auto &cached : pkg->second->bufferCache) {
        auto found = lineIdx->second.find(cached.first);
        if (found == lineIdx->second.end()) { continue; }
        auto signature = sliceFrom(cached.second, found->second);
        if (!signature.empty() && signature.find(identifier) != std::string::npos) {
            return signature;
        }
    }
    return {};
}

// LCOV_EXCL_START
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
// LCOV_EXCL_STOP
void CjdIndexer::BuildIndexFromCache()
{
    Trace::Log("BuildIndexFromCache Start");
    std::string cjdIndexDir = JoinPath(JoinPath(cjdCachePath, ".cache"), "index");
    std::map<int, std::vector<std::string>> fileMap;
    for (auto& idxFile:
            FileUtil::GetAllFilesUnderCurrentPath(cjdIndexDir, "idx")) {
        // LCOV_EXCL_START
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
        // LCOV_EXCL_STOP
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
    BuildSourceLineIndex();
    ParsePackageDependencies();
    BuildCJDIndex();
    pkgLineIndex.clear(); // no longer needed once the CJD index is built
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
    validFile.open(FileStore::NormalizePath(JoinPath(cjdCachePath, "valid.txt")));
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
