// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_FRONTEND_LSPCOMPILERINSTANCE_H
#define CANGJIE_FRONTEND_LSPCOMPILERINSTANCE_H

#include <utility>

#include "../json-rpc/StdioTransport.h"
#include "CjoManager.h"
#include "DependencyGraph.h"
#include "cangjie/Frontend/CompilerInstance.h"
#include "cangjie/Macro/MacroExpansion.h"
#include "capabilities/diagnostic/LSPDiagObserver.h"
#include "common/multiModule/ModuleManager.h"
#include "index/SymbolCollector.h"
#include "logger/Logger.h"

namespace Cangjie {
template <typename Func, typename... Args>
bool ExecuteCompilerApi(const std::string &name, Func func, Args &&...args)
{
    ark::Logger::Instance().CollectKernelLog(std::this_thread::get_id(), name, "start");
    try {
        Trace::Wlog("#### execute " + name + " start ####");
        std::invoke(func, std::forward<Args>(args)...);
        Trace::Wlog("#### execute " + name + " end ####");
    } catch (std::exception &e) {
        ark::Logger::Instance().LogMessage(ark::MessageType::MSG_ERROR,
            "Func " + name + e.what());
        return false;
    } catch (...) {
        ark::Logger::Instance().LogMessage(ark::MessageType::MSG_ERROR,
            "Func " + name + "Caught an unknown exception");
        return false;
    }
    ark::Logger::Instance().CollectKernelLog(std::this_thread::get_id(), name, "end");
    return true;
}

class PackageMapNode {
public:
    PackageMapNode() : inDegree(0) {}

    std::set<std::string> importPackages;
    std::set<std::string> downstreamPkgs;
    size_t inDegree;
    bool isInModule = true;
};

class LSPCompilerInstance : public CompilerInstance {
public:
    using PackageMap = std::unordered_map<std::string, PackageMapNode>;
    using CjoCacheMap = std::unordered_map<std::string, std::vector<uint8_t>>;
    LSPCompilerInstance(ark::Callbacks *cb,
                        CompilerInvocation &invocation,
                        DiagnosticEngine &diag,
                        std::string realPkgName,
                        const std::unique_ptr<ark::ModuleManager> &moduleManger);

    virtual ~LSPCompilerInstance() { callback = nullptr; }

    void PreCompileProcess();

    void CompilePassForComplete(const std::unique_ptr<ark::CjoManager> &cjoManager,
        const std::unique_ptr<ark::DependencyGraph> &graph,
        Position pos = INVALID_POSITION, const std::string &name = "");

    bool MacroExpand()
    {
        std::lock_guard<std::mutex> lock(ark::StdioTransport::Instance().stdoutMutex);
        const bool ret =
            ExecuteCompilerApi("PerformMacroExpand", &CompilerInstance::PerformMacroExpand, this);
        return ret;
    }

    static std::vector<std::string> GetTopologySort();

    static void SetCjoPathInModules(const std::string &cangjieHome, const std::string &cangjiePath);

    static void ReadCjoFileOneModule(const std::string &modulePath);

    static void ReadCjoFileOneModuleExternal(const std::string &modulesPath);

    static void UpdateUsrCjoFileCacheMap(
        std::string &moduleName, std::unordered_map<std::string, std::string> &CjoRequiresMap);

    static void InitCacheFileCacheMap();

    std::unordered_set<std::string> GetAllImportedCjo(
        const std::string &pkgName, std::unordered_map<std::string, bool> &isVisited);

    bool ToImportPackage(const std::string &curModuleName, const std::string &cjoPackage);

    bool Parse()
    {
        return ExecuteCompilerApi("PerformParse", &CompilerInstance::PerformParse, this);
    }

    bool ConditionCompile()
    {
        return ExecuteCompilerApi("PerformConditionCompile",
                                  &CompilerInstance::PerformConditionCompile, this);
    }

    bool ImportPackage()
    {
        return ExecuteCompilerApi("PerformImportPackage", &CompilerInstance::PerformImportPackage,
                                  this);
    }

    bool Sema() { return ExecuteCompilerApi("PerformSema", &CompilerInstance::PerformSema, this); }

    bool ExportAST(
        bool saveFileWithAbsPath,
        std::vector<uint8_t> &astData,
        const AST::Package &pkg,
        const std::function<void(ASTWriter &)> additionalSerializations = [](ASTWriter &) {})
    {
        return ExecuteCompilerApi("ExportAST", &ImportManager::ExportAST, this->importManager,
                                  saveFileWithAbsPath, astData, pkg, additionalSerializations);
    }

    virtual std::string Denoising(std::string candidate);

    Ptr<AST::File> GetFileByPath(const std::string& filePath) override;

    void ImportUsrPackage(const std::string &curModuleName);

    void ImportUsrCjo(const std::string &curModuleName);

    virtual void ImportCjoToManager(
        const std::unique_ptr<ark::CjoManager> &cjoManager, const std::unique_ptr<ark::DependencyGraph> &graph);

    void IndexCjoToManager(
        const std::unique_ptr<ark::CjoManager> &cjoManager, const std::unique_ptr<ark::DependencyGraph> &graph);

    bool CompileAfterParse(
        const std::unique_ptr<ark::CjoManager> &cjoManager, const std::unique_ptr<ark::DependencyGraph> &graph);

    std::unordered_map<std::string, ark::EdgeType> UpdateUpstreamPkgs();

    void UpdateDepGraph(bool isIncrement = true, const std::string &prePkgName = "");

    void UpdateDepGraph(const std::unique_ptr<ark::DependencyGraph> &graph, const std::string &prePkgName);

    ark::Callbacks *callback = nullptr;
    std::string pkgNameForPath; // Real Package Name
    std::string pkgNameForCj;
    bool macroExpandSuccess = false;
    std::set<std::string> upstreamPkgs;
    const std::unique_ptr<ark::ModuleManager> &moduleManger;

    static inline std::shared_mutex mtx;
    static inline PackageMap dependentPackageMap;
    static inline std::unordered_map<std::string, std::pair<std::vector<uint8_t>, bool>> astDataMap;
    static inline std::vector<std::string> cjoPathInModules;
    static inline CjoCacheMap cjoFileCacheMap;
    static inline std::unordered_map<std::string, std::vector<std::string>> cjoLibraryMap;
    // key: moduleName, value: CjoCacheMap
    static inline std::unordered_map<std::string, CjoCacheMap> usrCjoFileCacheMap;
    static inline std::unordered_set<std::string> cjoPathSet;

private:
    static void MarkBrokenDecls(AST::Package &pkg);
};
} // namespace Cangjie
#endif // CANGJIE_FRONTEND_LSPCOMPILERINSTANCE_H
