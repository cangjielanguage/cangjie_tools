// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_MODULEMANAGER_H
#define LSPSERVER_MODULEMANAGER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "cangjie/AST/Node.h"
#include "nlohmann/json.hpp"
#include "MultiModuleCommon.h"

namespace ark {
class ModuleManager {
public:
    ModuleManager(std::string mainModulePath, nlohmann::json multiModuleOption)
        : projectRootPath(std::move(mainModulePath)), multiModuleOption(std::move(multiModuleOption))
    {
    }

    ~ModuleManager() = default;

    void WorkspaceModeParser(const std::string &workspace = "");

    std::unordered_set<std::string> GetAllRequiresOneModule(
        const std::string &require, std::unordered_map<std::string, bool> &isVisited);

    void SetPackageRequires(const nlohmann::json &jsonData, const std::string &modulePath);

    void SetRequireAllPackages();

    std::string GetExpectedPkgName(const Cangjie::AST::File &file);

    std::string projectRootPath;
    nlohmann::json multiModuleOption;
    // key: modulePath, value: ModuleInfo
    std::unordered_map<std::string, ModuleInfo> moduleInfoMap;
    // key: moduleName, value: requires
    std::unordered_map<std::string, std::unordered_set<std::string>> requirePackages;
    // key: moduleName, value: allRequires
    std::unordered_map<std::string, std::unordered_set<std::string>> requireAllPackages;
    // key: moduleName, value: modulePaths with same moduleName
    std::unordered_map<std::string, std::vector<std::string>> duplicateModules;
};
} // namespace ark

#endif // LSPSERVER_MODULEMANAGER_H