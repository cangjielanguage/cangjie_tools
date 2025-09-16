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
class DCompilerInstance final : public LSPCompilerInstance {
public:
    explicit DCompilerInstance(
            ark::Callbacks *cb, CompilerInvocation &invocation, DiagnosticEngine &diag, const std::string& pkgName)
        : LSPCompilerInstance(cb, invocation, diag, pkgName, moduleMgr)
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

class CjdIndexer {
public:
    explicit CjdIndexer(Callbacks *cb,
                        const std::string& stdCjdPath,
                        const std::string& ohosCjdPath,
                        const std::string& cjdCachePath,
                        bool enablePackaged = true)
        : callback(cb),
          stdCjdPath(stdCjdPath),
          ohosCjdPath(ohosCjdPath),
          cjdCachePath(JoinPath(cjdCachePath, CANGJIE_VERSION)),
          enablePackaged(enablePackaged)
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

    SymbolLocation GetSymbolDeclaration(SymbolID id, const std::string& fullPkgName);

    CommentGroups GetSymbolComments(SymbolID id, const std::string& fullPkgName);

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
    std::unordered_map<std::string, std::unique_ptr<DCompilerInstance>> ciMap;
};
} // namespace lsp
} // namespace ark
#endif // LSPSERVER_INDEX_CJDINDEXER_H
