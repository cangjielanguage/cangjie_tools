// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_INDEX_CJDINDEXER_H
#define LSPSERVER_INDEX_CJDINDEXER_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "../CompilerCangjieProject.h"
#include "Symbol.h"
#include "cangjie/Basic/Version.h"

namespace Cangjie {
// LCOV_EXCL_START
class DCompilerInstance final : public LSPCompilerInstance {
public:
    explicit DCompilerInstance(ark::Callbacks *cb, CompilerInvocation &invocation,
        std::unique_ptr<DiagnosticEngine> diag, const std::string& pkgName)
        : LSPCompilerInstance(cb, invocation, std::move(diag), pkgName, moduleMgr)
    {
    }

    void ImportCjoToManager(const std::unique_ptr<ark::CjoManager> &cjoManager,
                            const std::unique_ptr<ark::DependencyGraph> &graph) override;

    std::string Denoising(std::string candidate) override;

private:
    std::unique_ptr<ark::ModuleManager> moduleMgr = nullptr;
};
} // namespace Cangjie

namespace ark {
namespace lsp {
using namespace Cangjie::FileUtil;

struct DPkgInfo : public PkgInfo {
    explicit DPkgInfo(const std::string &pkgPath,
                      const std::string &curModulePath,
                      const std::string &curModuleName,
                      Callbacks *callback)
        : PkgInfo(pkgPath, curModulePath, curModuleName, callback)
    {
        compilerInvocation->globalOptions.compileCjd = true;
        compilerInvocation->globalOptions.enableAddCommentToAst = true;
    }
};
// LCOV_EXCL_STOP
class CjdIndexer {
public:
    explicit CjdIndexer(Callbacks *cb,
                        const std::string& stdCjdPath,
                        const std::string& ohosCjdPath,
                        const std::string& cjdCachePath,
                        bool enablePackaged = true)
        : enablePackaged(enablePackaged),
          stdCjdPath(stdCjdPath),
          ohosCjdPath(ohosCjdPath),
          cjdCachePath(JoinPath(cjdCachePath, CANGJIE_VERSION)),
          callback(cb)
    {
        if (CreateDirs(this->cjdCachePath) == -1) {
            Trace::Log("cjd cache dir build failed");
        }
        cacheManager = std::make_unique<CacheManager>(this->cjdCachePath);
        cacheManager->InitDir();
    }

    ~CjdIndexer() = default;

    static void InitInstance(Callbacks *cb, const std::string& stdCjdPathOption,
                             const std::string& ohosCjdPathOption, const std::string& cjdCachePathOption);

    static void DeleteInstance()
    {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }

    static CjdIndexer *GetInstance();

    // Looks up the CJD symbol by id in the given package and returns it, or nullptr if absent.
    // Works on both the fresh-build and cache-restore paths, since it reads pkgSymsMap.
    // Callers must ensure the indexer has finished building (GetRunningState() == false) so that
    // pkgSymsMap is not concurrently mutated while the returned pointer is in use.
    const Symbol *GetSymbol(SymbolID id, const std::string& fullPkgName);

    // Extracts the declaration signature text from the cached CJD source.
    // Used while indexing the CJD packages themselves, where pkgMap is populated.
    // `filePath` is an optional hint to locate the exact source buffer; when it does not match,
    // fall back to scanning all buffers of the package (correctness is preserved).
    std::string ExtractSignatureFromSource(const std::string& identifier, const Position& begin,
                                           const Position& end, const std::string& fullPkgName,
                                           const std::string& filePath = "");

    std::unordered_map<std::string, std::unique_ptr<DPkgInfo>> &GetPkgMap()
    {
        return pkgMap;
    }

    bool CheckCjdCache();

    void Build();

    void BuildIndexFromCache();

    bool GetRunningState() const
    {
        return isIndexing;
    }

private:
    // Precomputes per-line byte offsets for every cached CJD source file, so signature extraction
    // no longer re-splits a whole file per symbol. Must be called once (single-threaded) after
    // LoadAllCJDResource and before the concurrent BuildCJDIndex.
    void BuildSourceLineIndex();

    void ReadCJDSource(const std::string &rootPath, const std::string &modulePath,
                       std::map<int, std::vector<std::string>> &fileMap, const std::string &parentPkg = "");

    void LoadAllCJDResource();

    void ReadPackagedCjdResource(const std::string& rootPath, const std::string& filePath,
        std::map<int, std::vector<std::string>> &fileMap);

    void ParsePackageDependencies();

    void BuildCJDIndex();

    void GenerateValidFile();

    std::string GetValidCode();

    static CjdIndexer *instance;
    std::mutex mtx;
    bool enablePackaged = true;
    bool isIndexing = false;
    std::string cangjieHome;
    std::string stdCjdPath;
    std::string ohosCjdPath;
    std::string cjdCachePath;
    Callbacks *callback = nullptr;
    std::unique_ptr<DependencyGraph> graph = std::make_unique<DependencyGraph>();
    std::unique_ptr<CjoManager> cjoManager = std::make_unique<CjoManager>();
    std::unique_ptr<ThrdPool> thrdPool = std::make_unique<ThrdPool>(6);
    std::unique_ptr<CacheManager> cacheManager;

    std::map<std::string, SymbolSlab> pkgSymsMap{};
    std::unordered_map<std::string, std::unique_ptr<DPkgInfo>> pkgMap;
    // Per package -> per source file -> line-start byte offsets (index into the file content
    // stored in pkgMap's bufferCache). Built once by BuildSourceLineIndex, then read-only.
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<size_t>>> pkgLineIndex;
    std::unordered_map<std::string, std::unique_ptr<DCompilerInstance>> ciMap;
};
} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_CJDINDEXER_H
