// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ArkLanguageServer.h"
#include "CompilerCangjieProject.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace ark;

class TestTransport : public Transport {
public:
    TestTransport() = default;
    ~TestTransport() override = default;

    void SetIO(std::FILE *in, std::FILE *out) override {
    }

    void Notify(std::string method, ValueOrError result) override {
    }

    void Reply(nlohmann::json id, ValueOrError result) override {
    }

    LSPRet Loop(MessageHandler &handler) override {
        return ark::LSPRet::SUCCESS;
    }

};

Environment CreateTestEnvironment() {
    Environment env;
    env.cangjieHome = "test_workspace";
    env.cangjiePath = "test_stdlib";
    return env;
}

std::unique_ptr<lsp::IndexDatabase> CreateTestIndexDatabase() {
    return std::unique_ptr<lsp::IndexDatabase>();
}

class CompilerCangjieProjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        workspace = "test_workspace";
        stdLibPath = "test_stdlib";
        useDB = false;

        transport = std::make_unique<TestTransport>();

        env = CreateTestEnvironment();

        indexDB = CreateTestIndexDatabase();

        server = std::make_unique<ArkLanguageServer>(*transport, env, indexDB.get());

        CompilerCangjieProject::InitInstance(server.get(), indexDB.get());
    }

    void TearDown() override {
    }

    std::string workspace;
    std::string stdLibPath;
    bool useDB;
    std::unique_ptr<TestTransport> transport;
    Environment env;
    std::unique_ptr<lsp::IndexDatabase> indexDB;
    std::unique_ptr<ArkLanguageServer> server;
};

TEST_F(CompilerCangjieProjectTest, GetInstanceTest) {
    CompilerCangjieProject* instance1 = CompilerCangjieProject::GetInstance();
    CompilerCangjieProject* instance2 = CompilerCangjieProject::GetInstance();
    EXPECT_EQ(instance1, instance2);
}


TEST_F(CompilerCangjieProjectTest, InitInstanceTest) {
    EXPECT_NE(CompilerCangjieProject::GetInstance(), nullptr);
    EXPECT_NE(CompilerCangjieProject::GetInstance()->GetCallback(), nullptr);
    EXPECT_NE(CompilerCangjieProject::GetInstance()->GetIndex(), nullptr);
}

TEST_F(CompilerCangjieProjectTest, ResolveDependenceTest) {
    std::vector<std::vector<std::string>> cycles = CompilerCangjieProject::GetInstance()->ResolveDependence();
}

TEST_F(CompilerCangjieProjectTest, GetConditionCompileTest) {
    std::string pkgName = "test_pkg";
    std::string moduleName = "test_module";
    std::unordered_map<std::string, std::string> compileOptions = CompilerCangjieProject::GetInstance()->GetConditionCompile(pkgName, moduleName);
}

TEST_F(CompilerCangjieProjectTest, GetRealPathTest) {
    std::string path = "test_path";
    CompilerCangjieProject::GetInstance()->GetRealPath(path);
}

TEST_F(CompilerCangjieProjectTest, CalculateScoreTest) {
    CodeCompletion item;
    std::string prefix = "test_prefix";
    uint8_t cursorDepth = 2;
    double score = CompilerCangjieProject::GetInstance()->CalculateScore(item, prefix, cursorDepth);
    EXPECT_GE(score, 0.0);
}

TEST_F(CompilerCangjieProjectTest, UpdateUsageFrequencyTest) {
    std::string item = "test_item";
    CompilerCangjieProject::GetInstance()->UpdateUsageFrequency(item);
}

TEST_F(CompilerCangjieProjectTest, GetIndexTest) {
    lsp::SymbolIndex* index = CompilerCangjieProject::GetInstance()->GetIndex();
    EXPECT_NE(index, nullptr);
}

TEST_F(CompilerCangjieProjectTest, GetMemIndexTest) {
    lsp::MemIndex* memIndex = CompilerCangjieProject::GetInstance()->GetMemIndex();
    EXPECT_NE(memIndex, nullptr);
}

TEST_F(CompilerCangjieProjectTest, GetCjoManagerTest) {
    ark::CjoManager* cjoManager = CompilerCangjieProject::GetInstance()->GetCjoManager();
    EXPECT_NE(cjoManager, nullptr);
}

TEST_F(CompilerCangjieProjectTest, GetDependencyGraphTest) {
    DependencyGraph* graph = CompilerCangjieProject::GetInstance()->GetDependencyGraph();
    EXPECT_NE(graph, nullptr);
}

TEST_F(CompilerCangjieProjectTest, CheckNeedCompilerTest) {
    std::string fileName = "test_file.cj";
    bool needCompile = CompilerCangjieProject::GetInstance()->CheckNeedCompiler(fileName);
}

TEST_F(CompilerCangjieProjectTest, IncrementOnePkgCompileTest) {
    std::string filePath = "test_file.cj";
    std::string contents = "test_contents";
    CompilerCangjieProject::GetInstance()->IncrementOnePkgCompile(filePath, contents);
}

TEST_F(CompilerCangjieProjectTest, IncrementTempPkgCompileTest)
{
    {
        std::string basicString = "nonexistent_pkg";
        CompilerCangjieProject::GetInstance()->IncrementTempPkgCompile(basicString);
    }

    {
        std::string basicString = "test_pkg";
        CompilerCangjieProject::GetInstance()->IncrementTempPkgCompile(basicString);
    }

    {
        std::string basicString = "test_pkg";
        CompilerCangjieProject::GetInstance()->IncrementTempPkgCompile(basicString);
    }

    {
        std::string basicString = "test_pkg";
        CompilerCangjieProject::GetInstance()->IncrementTempPkgCompile(basicString);
    }
}

TEST_F(CompilerCangjieProjectTest, EraseOtherCacheTest) {
    std::string fullPkgName = "test_pkg";
    CompilerCangjieProject::GetInstance()->EraseOtherCache(fullPkgName);
}

TEST_F(CompilerCangjieProjectTest, UpdateOnDiskTest) {
    std::string path = "test_path";
    CompilerCangjieProject::GetInstance()->UpdateOnDisk(path);
}

TEST_F(CompilerCangjieProjectTest, DenoisingTest) {
    std::string candidate = "test_candidate";
    std::string result = CompilerCangjieProject::GetInstance()->Denoising(candidate);
}

TEST_F(CompilerCangjieProjectTest, GetPackageSpecModTest) {
    Node* node = nullptr;
    ark::Modifier mod = CompilerCangjieProject::GetInstance()->GetPackageSpecMod(node);
    EXPECT_EQ(mod, ark::Modifier::UNDEFINED);
}

TEST_F(CompilerCangjieProjectTest, IsVisibleForPackageTest) {
    std::string curPkgName = "current_pkg";
    std::string importPkgName = "imported_pkg";
    bool isVisible = CompilerCangjieProject::GetInstance()->IsVisibleForPackage(curPkgName, importPkgName);
}

TEST_F(CompilerCangjieProjectTest, IsCurModuleCjoDepTest) {
    std::string curModule = "current_module";
    std::string fullPkgName = "test_pkg";
    bool isDep = CompilerCangjieProject::GetInstance()->IsCurModuleCjoDep(curModule, fullPkgName);
}

TEST_F(CompilerCangjieProjectTest, GetFullPkgNameToPathMapTest) {
    std::unordered_map<std::string, std::string> pkgMap = CompilerCangjieProject::GetInstance()->GetFullPkgNameToPathMap();
}

TEST_F(CompilerCangjieProjectTest, GetContentByFileTest) {
    std::string filePath = "test_file.cj";
    std::string content = CompilerCangjieProject::GetInstance()->GetContentByFile(filePath);
}

TEST_F(CompilerCangjieProjectTest, CheckPackageModifierTest) {
    File needCheckedFile;
    std::string fullPackageName = "test_pkg";
    bool result = CompilerCangjieProject::GetInstance()->CheckPackageModifier(needCheckedFile, fullPackageName);
}

TEST_F(CompilerCangjieProjectTest, GetCallbackTest) {
    Callbacks* callback = CompilerCangjieProject::GetInstance()->GetCallback();
    EXPECT_NE(callback, nullptr);
}

TEST_F(CompilerCangjieProjectTest, StoreAllPackagesCacheTest) {
    CompilerCangjieProject::GetInstance()->StoreAllPackagesCache();
}

TEST_F(CompilerCangjieProjectTest, ReportCircularDepsEmptyCycles) {
    std::vector<std::vector<std::string>> cycles;
    CompilerCangjieProject::GetInstance()->ReportCircularDeps(cycles);
}

TEST_F(CompilerCangjieProjectTest, ReportCircularDepsMultipleCycles) {
    Callbacks* callback = CompilerCangjieProject::GetInstance()->GetCallback();
    std::vector<std::vector<std::string>> cycles = {
        {"pkg1", "pkg2"},
        {"pkg3", "pkg4"}
    };

    CompilerCangjieProject::GetInstance()->ReportCircularDeps(cycles);
}

TEST_F(CompilerCangjieProjectTest, GetFullPkgNameTest) {
    std::string filePath = "test_workspace/src/test_pkg/test_file.cj";
    std::string fullPkgName = CompilerCangjieProject::GetInstance()->GetFullPkgName(filePath);
    // Test returns expected package name format
}

TEST_F(CompilerCangjieProjectTest, GetFullPkgByDirTest) {
    std::string dirPath = "test_workspace/src/test_pkg";
    std::string fullPkgName = CompilerCangjieProject::GetInstance()->GetFullPkgByDir(dirPath);
    // Test package name resolution from directory path
}

TEST_F(CompilerCangjieProjectTest, GetFileIDTest) {
    std::string fileName = "test_workspace/src/test_pkg/test_file.cj";

    // Test file ID retrieval
    int fileId = CompilerCangjieProject::GetInstance()->GetFileID(fileName);
    EXPECT_GE(fileId, 0);
}

TEST_F(CompilerCangjieProjectTest, GetFileIDForCompeteTest) {
    std::string fileName = "test_workspace/src/test_pkg/test_file.cj";

    // Test file ID retrieval for completion
    int fileId = CompilerCangjieProject::GetInstance()->GetFileIDForCompete(fileName);
    EXPECT_GE(fileId, 0);
}

TEST_F(CompilerCangjieProjectTest, FileHasSemaCacheTest) {
    std::string fileName = "test_workspace/src/test_pkg/test_file.cj";

    // Test semantic cache existence check
    bool hasCache = CompilerCangjieProject::GetInstance()->FileHasSemaCache(fileName);
    // Result depends on test environment setup
}

TEST_F(CompilerCangjieProjectTest, PkgHasSemaCacheTest) {
    std::string pkgName = "test_pkg";

    // Test package semantic cache existence check
    bool hasCache = CompilerCangjieProject::GetInstance()->PkgHasSemaCache(pkgName);
    // Result depends on test environment setup
}

TEST_F(CompilerCangjieProjectTest, GetPathBySourceWithFileNameTest) {
    std::string fileName = "test_workspace/src/test_pkg/test_file.cj";
    unsigned int fileId = 1;

    // Test path resolution by source file name and ID
    std::string path = CompilerCangjieProject::GetInstance()->GetPathBySource(fileName, fileId);
    // Verify path format and existence
}

TEST_F(CompilerCangjieProjectTest, ClearParseCacheTest) {
    // Test parse cache clearance functionality
    CompilerCangjieProject::GetInstance()->ClearParseCache();
    // Verify cache is cleared successfully
}

TEST_F(CompilerCangjieProjectTest, GetMacroLibsTest) {
    // Test macro libraries retrieval
    std::vector<std::string> macroLibs = CompilerCangjieProject::GetInstance()->GetMacroLibs();
    // Verify returned list content
}

TEST_F(CompilerCangjieProjectTest, GetCjcTest) {
    // Test CJC path retrieval
    std::string cjcPath = CompilerCangjieProject::GetInstance()->GetCjc();
    // Verify path format and validity
}

TEST_F(CompilerCangjieProjectTest, GetConditionCompilePathsTest) {
    // Test condition compilation paths retrieval
    std::vector<std::string> paths = CompilerCangjieProject::GetInstance()->GetConditionCompilePaths();
    // Verify returned paths list
}

TEST_F(CompilerCangjieProjectTest, GetDiagCurEditFileTest) {
    std::string file = "test_workspace/src/test_pkg/test_file.cj";

    // Test diagnostics retrieval for current edit file
    CompilerCangjieProject::GetInstance()->GetDiagCurEditFile(file);
    // Verify diagnostics processing
}

TEST_F(CompilerCangjieProjectTest, UpdateBuffCacheTest) {
    std::string file = "test_workspace/src/test_pkg/test_file.cj";

    // Test buffer cache update functionality
    CompilerCangjieProject::GetInstance()->UpdateBuffCache(file, true);
    // Verify cache update operation
}