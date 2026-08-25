// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "common/FileStore.h"
#include "common/multiModule/ModuleManager.h"
#include "URI.h"

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

#include "gtest/gtest.h"

using namespace ark;

namespace {
class ModuleManagerTest : public testing::Test {
protected:
    void SetUp() override
    {
        root = fs::temp_directory_path() / "cangjie-lsp-module-manager-test";
        fs::remove_all(root);
        workspace = root / "workspace";
        internalBoo = workspace / "boo";
        externalBoo = root / "ddd" / "boo";
        fs::create_directories(internalBoo / "src");
        fs::create_directories(externalBoo / "src");
    }

    void TearDown() override
    {
        fs::remove_all(root);
    }

    static std::string ToUri(const fs::path &path)
    {
        return URI::URIFromAbsolutePath(path.string()).ToString();
    }

    nlohmann::json CreateModuleOption(const std::string &internalName = "boo",
                                      const std::string &externalName = "boo") const
    {
        nlohmann::json option;
        option[ToUri(internalBoo)] = {
            {"name", internalName},
            {"requires", {{externalName, {{"path", ToUri(externalBoo)}}}}}
        };
        option[ToUri(externalBoo)] = {{"name", externalName}, {"requires", nlohmann::json::object()}};
        return option;
    }

    fs::path root;
    fs::path workspace;
    fs::path internalBoo;
    fs::path externalBoo;
};
}

TEST_F(ModuleManagerTest, ReportsConflictOnlyForModuleInsideWorkspace)
{
    ModuleManager manager(FileStore::NormalizePath(workspace.string()), CreateModuleOption());
    manager.WorkspaceModeParser();

    ModuleNameConflict conflict;
    EXPECT_TRUE(manager.GetModuleNameConflict((internalBoo / "src" / "main.cj").string(), conflict));
    EXPECT_EQ(conflict.moduleName, "boo");
    ASSERT_EQ(conflict.modulePaths.size(), 2U);
    EXPECT_FALSE(manager.GetModuleNameConflict((externalBoo / "src" / "main.cj").string(), conflict));
}

TEST_F(ModuleManagerTest, DoesNotReportDifferentModuleNames)
{
    ModuleManager manager(FileStore::NormalizePath(workspace.string()), CreateModuleOption("boo", "doo"));
    manager.WorkspaceModeParser();

    ModuleNameConflict conflict;
    EXPECT_FALSE(manager.GetModuleNameConflict((internalBoo / "src" / "main.cj").string(), conflict));
}

TEST_F(ModuleManagerTest, DoesNotReportSameCanonicalModulePath)
{
    nlohmann::json option;
    option[ToUri(internalBoo)] = {
        {"name", "boo"},
        {"requires", {{"boo", {{"path", ToUri(internalBoo / ".." / "boo")}}}}}
    };
    option[ToUri(internalBoo / ".." / "boo")] = {{"name", "boo"}};
    ModuleManager manager(FileStore::NormalizePath(workspace.string()), option);
    manager.WorkspaceModeParser();

    ModuleNameConflict conflict;
    EXPECT_FALSE(manager.GetModuleNameConflict((internalBoo / "src" / "main.cj").string(), conflict));
}

TEST_F(ModuleManagerTest, DoesNotReportUnrelatedModulesWithSameName)
{
    nlohmann::json option;
    option[ToUri(internalBoo)] = {{"name", "boo"}, {"requires", nlohmann::json::object()}};
    option[ToUri(externalBoo)] = {{"name", "boo"}, {"requires", nlohmann::json::object()}};
    ModuleManager manager(FileStore::NormalizePath(workspace.string()), option);
    manager.WorkspaceModeParser();

    ModuleNameConflict conflict;
    EXPECT_FALSE(manager.GetModuleNameConflict((internalBoo / "src" / "main.cj").string(), conflict));
}

TEST_F(ModuleManagerTest, DoesNotReportDependencyWhoseActualModuleNameDiffers)
{
    nlohmann::json option = CreateModuleOption("boo", "doo");
    option[ToUri(internalBoo)]["requires"] = {{"boo", {{"path", ToUri(externalBoo)}}}};
    ModuleManager manager(FileStore::NormalizePath(workspace.string()), option);
    manager.WorkspaceModeParser();

    ModuleNameConflict conflict;
    EXPECT_FALSE(manager.GetModuleNameConflict((internalBoo / "src" / "main.cj").string(), conflict));
}

TEST_F(ModuleManagerTest, DoesNotReportScriptDependencyAsSourceModuleConflict)
{
    nlohmann::json option = CreateModuleOption();
    option[ToUri(internalBoo)]["requires"]["boo"]["isScriptDependence"] = true;
    ModuleManager manager(FileStore::NormalizePath(workspace.string()), option);
    manager.WorkspaceModeParser();

    ModuleNameConflict conflict;
    EXPECT_FALSE(manager.GetModuleNameConflict((internalBoo / "src" / "main.cj").string(), conflict));
}

TEST_F(ModuleManagerTest, DoesNotReportNestedDependencyWithoutSameNameEdge)
{
    auto nestedBoo = workspace / "third_party" / "boo";
    fs::create_directories(nestedBoo / "src");
    nlohmann::json option;
    option[ToUri(internalBoo)] = {{"name", "boo"}, {"requires", nlohmann::json::object()}};
    option[ToUri(nestedBoo)] = {{"name", "boo"}, {"requires", nlohmann::json::object()}};
    ModuleManager manager(FileStore::NormalizePath(workspace.string()), option);
    manager.WorkspaceModeParser();

    ModuleNameConflict conflict;
    EXPECT_FALSE(manager.GetModuleNameConflict((internalBoo / "src" / "main.cj").string(), conflict));
    EXPECT_FALSE(manager.GetModuleNameConflict((nestedBoo / "src" / "main.cj").string(), conflict));
}
