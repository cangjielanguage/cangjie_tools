// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "CompilerCangjieProject.h"
#include "capabilities/diagnostic/InternalImportDiag.h"
#include "capabilities/diagnostic/LSPDiagObserver.h"

#if defined(__has_include)
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace {
using namespace ark;
using namespace Cangjie;
using namespace Cangjie::AST;
constexpr int UNRELATED_DIAGNOSTIC_CODE = 123;
constexpr int INTERNAL_FILE_IMPORT_END_COLUMN = sizeof("internal import std.fs.File") - 1;

class ImportDiagnosticCallbacks : public Callbacks {
public:
    std::map<std::string, std::set<DiagnosticToken, DiagnosticCompare>> diagnostics;

    std::vector<DiagnosticToken> GetDiagsOfCurFile(std::string file) override
    {
        const auto &items = diagnostics[file];
        return {items.begin(), items.end()};
    }
    void UpdateDiagnostic(std::string file, DiagnosticToken diagnostic) override
    {
        diagnostics[file].insert(std::move(diagnostic));
    }
    void RemoveDiagnostic(std::string file, DiagnosticToken diagnostic) override
    {
        diagnostics[file].erase(diagnostic);
    }
    void RemoveDiagOfCurFile(const std::string &file) override { diagnostics.erase(file); }
    void RemoveDiagOfCurPkg(const std::string &) override {}
    void RemoveUnusedDiagsOfCurFile(const std::string &) override {}
    void ReadyForDiagnostics(std::string, int64_t, std::vector<DiagnosticToken>) override {}
    std::string GetContentsByFile(const std::string &) override { return {}; }
    int64_t GetVersionByFile(const std::string &) override { return 1; }
    void RemoveDocByFile(const std::string &) override {}
    bool NeedReParser(const std::string &) override { return false; }
    void UpdateDocNeedReparse(const std::string &, int64_t, bool) override {}
    void AddDocWhenInitCompile(const std::string &) override {}
    void ReportCjoVersionErr(std::string) override {}
    void PublishCompletionTip(const CompletionTip &) override {}
    std::string CreateProgressToken() override { return {}; }
    void SendWorkDoneProgressCreate(const WorkDoneProgressCreateParams &) override {}
    void SendWorkDoneProgressBegin(const std::string &, const WorkDoneProgressBegin &) override {}
    void SendWorkDoneProgressReport(const std::string &, const WorkDoneProgressReport &) override {}
    void SendWorkDoneProgressEnd(const std::string &, const WorkDoneProgressEnd &) override {}
};

class ImportDiagnosticCompiler : public CompilerInstance {
public:
    ImportDiagnosticCompiler(CompilerInvocation &invocation, DiagnosticEngine &diag)
        : CompilerInstance(invocation, diag) {}
    bool PerformParse() override
    {
        ++parseCount;
        return CompilerInstance::PerformParse();
    }

    bool PerformImportPackage() override
    {
        ++importCount;
        return CompilerInstance::PerformImportPackage();
    }

    bool PerformSema() override
    {
        ++semaCount;
        return CompilerInstance::PerformSema();
    }
    unsigned int parseCount = 0;
    unsigned int importCount = 0;
    unsigned int semaCount = 0;
};

class InternalImportDiagTest : public testing::Test {
protected:
    void SetUp() override
    {
        const auto *sdk = std::getenv("CANGJIE_HOME");
        ASSERT_NE(sdk, nullptr) << "Load the Cangjie SDK before running this test.";
        sdkHome = sdk;
        const auto id = std::chrono::steady_clock::now().time_since_epoch().count();
        testRoot = fs::temp_directory_path() / ("lsp_internal_import_diag_" + std::to_string(id));
        fs::create_directories(testRoot / "src");
        invocation.globalOptions.moduleName = "src";
        invocation.globalOptions.compilePackage = true;
        invocation.globalOptions.packagePaths = {(testRoot / "src").generic_string()};
        invocation.globalOptions.noSubPkg = false;
        invocation.globalOptions.outputMode = GlobalOptions::OutputMode::STATIC_LIB;
        invocation.globalOptions.enableMacroInLSP = true;
        invocation.globalOptions.ReadPathsFromEnvironmentVars({{"CANGJIE_HOME", sdkHome}});
        diag.RegisterHandler(std::make_unique<LSPDiagObserver>(&callbacks, diag));
    }

    void TearDown() override
    {
        compiler.reset();
        if (!testRoot.empty()) {
            fs::remove_all(testRoot);
        }
    }

    std::string CollectErrorMessages() const
    {
        std::string errors;
        for (const auto &entry : callbacks.diagnostics) {
            for (const auto &diagnostic : entry.second) {
                if (diagnostic.severity == static_cast<int>(DiagLSPSeverity::ERROR_DIAG)) {
                    errors += diagnostic.message + "\n";
                }
            }
        }
        return errors;
    }

    void Compile(const std::map<std::string, std::string> &files, bool expectErrors = false)
    {
        compiler = std::make_unique<ImportDiagnosticCompiler>(invocation, diag);
        compiler->cangjieHome = sdkHome;
        compiler->loadSrcFilesFromCache = true;
        compiler->importManager->SetSourceCodeImportStatus(false);
        for (const auto &[name, content] : files) {
            const auto path = Path(name);
            std::ofstream stream(path);
            ASSERT_TRUE(stream.good());
            stream << content;
            compiler->bufferCache[path] = CompilerInstance::SrcCodeCacheInfo(content);
        }
        ASSERT_TRUE(compiler->PerformParse());
        ASSERT_TRUE(compiler->PerformConditionCompile());
        const bool imported = compiler->PerformImportPackage();
        const bool checked = compiler->PerformSema();
        if (expectErrors) {
            ASSERT_GT(diag.GetErrorCount(), 0u);
        } else {
            const auto errors = CollectErrorMessages();
            ASSERT_TRUE(imported) << errors;
            ASSERT_TRUE(checked) << errors;
            ASSERT_EQ(diag.GetErrorCount(), 0u);
        }
        ASSERT_EQ(compiler->GetSourcePackages().size(), 1u);
        package = compiler->GetSourcePackages().front();
        ASSERT_NE(package, nullptr);
    }

    std::string Path(const std::string &name) const
    {
        auto path = testRoot;
        path.append("src");
        path.append(name);
        return path.generic_string();
    }

    void Refresh(bool noSubPkg)
    {
        InternalImportDiag::Refresh(*package, *compiler->importManager, diag, callbacks, noSubPkg);
    }

    std::vector<DiagnosticToken> Unused(const std::string &name)
    {
        auto result = callbacks.GetDiagsOfCurFile(Path(name));
        result.erase(std::remove_if(result.begin(), result.end(), [](const auto &item) {
                return !item.diagFix || !item.diagFix->removeImport;
            }), result.end());
        return result;
    }

    ImportDiagnosticCallbacks callbacks;
    CompilerInvocation invocation;
    DiagnosticEngine diag;
    std::unique_ptr<ImportDiagnosticCompiler> compiler;
    Ptr<Package> package;
    fs::path testRoot;
    std::string sdkHome;
};

TEST_F(InternalImportDiagTest, RefreshesCachedDiagnosticsWithoutCompilingOrChangingExports)
{
    Compile({{"main.cj", "package src\ninternal import std.fs.File\nmain(): Int64 { 0 }\n"}});
    ASSERT_FALSE(HasFatalFailure());
    const auto *originalPackage = package.get();
    const auto originalImport = package->files.front()->imports.front().get();
    const auto originalNoSubPkg = package->noSubPkg;
    std::vector<uint8_t> before;
    compiler->importManager->ExportAST(false, before, *package);
    ASSERT_FALSE(before.empty());
    EXPECT_TRUE(Unused("main.cj").empty());

    Refresh(true);
    auto diagnostics = Unused("main.cj");
    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics.front().tags, std::vector<int>{1});
    EXPECT_EQ(diagnostics.front().severity, static_cast<int>(DiagLSPSeverity::HINT));
    EXPECT_EQ(diagnostics.front().range.start.line, 1);
    EXPECT_EQ(diagnostics.front().range.start.column, 0);
    EXPECT_EQ(diagnostics.front().range.end.column, INTERNAL_FILE_IMPORT_END_COLUMN);
    Refresh(true);
    EXPECT_EQ(Unused("main.cj").size(), 1u);
    Refresh(false);
    EXPECT_TRUE(Unused("main.cj").empty());
    Refresh(true);
    EXPECT_EQ(Unused("main.cj").size(), 1u);

    std::vector<uint8_t> after;
    compiler->importManager->ExportAST(false, after, *package);
    EXPECT_EQ(after, before);
    EXPECT_EQ(package.get(), originalPackage);
    EXPECT_EQ(package->files.front()->imports.front().get(), originalImport);
    EXPECT_EQ(package->noSubPkg, originalNoSubPkg);
    EXPECT_EQ(compiler->parseCount, 1u);
    EXPECT_EQ(compiler->importCount, 1u);
    EXPECT_EQ(compiler->semaCount, 1u);
}

TEST_F(InternalImportDiagTest, PreservesPrivateImportAndUnrelatedDiagnostics)
{
    Compile({{"main.cj", "package src\ninternal import std.fs.File\nimport std.fs.Directory\nmain(): Int64 { 0 }\n"}});
    ASSERT_FALSE(HasFatalFailure());
    ASSERT_EQ(Unused("main.cj").size(), 1u);
    DiagnosticToken unrelated;
    unrelated.code = UNRELATED_DIAGNOSTIC_CODE;
    unrelated.severity = 1;
    unrelated.message = "preserved diagnostic";
    unrelated.range = {{0, 9, 0}, {0, 9, 1}};
    callbacks.UpdateDiagnostic(Path("main.cj"), unrelated);
    Refresh(true);
    EXPECT_EQ(Unused("main.cj").size(), 2u);
    Refresh(false);
    auto remaining = callbacks.GetDiagsOfCurFile(Path("main.cj"));
    ASSERT_EQ(remaining.size(), 2u);
    EXPECT_EQ(Unused("main.cj").size(), 1u);
    EXPECT_TRUE(std::any_of(remaining.begin(), remaining.end(), [](const auto &item) {
        return item.message == "preserved diagnostic";
    }));
}

TEST_F(InternalImportDiagTest, KeepsAliasUsedFromAnotherFile)
{
    Compile({
        {"main.cj", "package src\ninternal import std.fs.File as InputFile\nmain(): Int64 { 0 }\n"},
        {"helper.cj", "package src\npublic func keep(value: InputFile): InputFile { value }\n"}
    });
    ASSERT_FALSE(HasFatalFailure());
    Refresh(true);
    EXPECT_TRUE(Unused("main.cj").empty());
}

TEST_F(InternalImportDiagTest, ReportsUnusedAliasAndWildcardPreservingPrivatePackageImport)
{
    Compile({
        {"main.cj", "package src\n"
                    "internal import std.fs.File as InputFile\n"
                    "internal import std.collection.*\n"
                    "import std.time\n"
                    "main(): Int64 { 0 }\n"}
    });
    ASSERT_FALSE(HasFatalFailure());
    Refresh(true);
    EXPECT_EQ(Unused("main.cj").size(), 3u);
    Refresh(false);
    EXPECT_EQ(Unused("main.cj").size(), 1u);
}

TEST_F(InternalImportDiagTest, KeepsUsedWildcardAndPackageImports)
{
    Compile({
        {"main.cj", "package src\n"
                    "internal import std.fs.*\n"
                    "import std.collection\n"
                    "public func keep(value: File): File { value }\n"
                    "main(): Int64 { collection.ArrayList<Int64>().size }\n"}
    });
    ASSERT_FALSE(HasFatalFailure());
    Refresh(true);
    EXPECT_TRUE(Unused("main.cj").empty());
}

TEST_F(InternalImportDiagTest, RefreshesAllParentFiles)
{
    Compile({
        {"main.cj", "package src\ninternal import std.fs.File\nmain(): Int64 { 0 }\n"},
        {"helper.cj", "package src\ninternal import std.fs.Directory\npublic func marker(): Int64 { 1 }\n"}
    });
    ASSERT_FALSE(HasFatalFailure());
    Refresh(true);
    EXPECT_EQ(Unused("main.cj").size(), 1u);
    EXPECT_EQ(Unused("helper.cj").size(), 1u);
    Refresh(false);
    EXPECT_TRUE(Unused("main.cj").empty());
    EXPECT_TRUE(Unused("helper.cj").empty());
}

TEST_F(InternalImportDiagTest, PreservesUnresolvedImportErrors)
{
    Compile({
        {"main.cj", "package src\n"
                    "internal import missing_internal_import_package.Missing\n"
                    "main(): Int64 { 0 }\n"}
        }, true);
    ASSERT_FALSE(HasFatalFailure());
    const auto before = callbacks.GetDiagsOfCurFile(Path("main.cj"));
    ASSERT_FALSE(before.empty());
    Refresh(true);
    const auto after = callbacks.GetDiagsOfCurFile(Path("main.cj"));
    ASSERT_EQ(after.size(), before.size());
    EXPECT_TRUE(Unused("main.cj").empty());
    for (size_t i = 0; i < before.size(); ++i) {
        EXPECT_EQ(after[i].message, before[i].message);
        EXPECT_EQ(after[i].severity, before[i].severity);
        EXPECT_EQ(after[i].range, before[i].range);
    }
}

TEST_F(InternalImportDiagTest, PreservesInvalidInternalPackageImportError)
{
    Compile({{"main.cj", "package src\ninternal import std.time\nmain(): Int64 { 0 }\n"}}, true);
    ASSERT_FALSE(HasFatalFailure());
    const auto before = callbacks.GetDiagsOfCurFile(Path("main.cj"));
    ASSERT_FALSE(before.empty());
    ASSERT_TRUE(std::any_of(before.begin(), before.end(), [](const auto &item) {
        return item.message.find("cannot be modified by 'internal'") != std::string::npos;
    }));
    Refresh(true);
    const auto after = callbacks.GetDiagsOfCurFile(Path("main.cj"));
    ASSERT_EQ(after.size(), before.size());
    EXPECT_TRUE(Unused("main.cj").empty());
    for (size_t i = 0; i < before.size(); ++i) {
        EXPECT_EQ(after[i].message, before[i].message);
        EXPECT_EQ(after[i].severity, before[i].severity);
        EXPECT_EQ(after[i].range, before[i].range);
    }
}
} // namespace
