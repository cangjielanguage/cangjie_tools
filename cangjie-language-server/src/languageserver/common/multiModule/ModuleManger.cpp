// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "ModuleManager.h"

#include "cangjie/Utils/FileUtil.h"
#include "../Constants.h"
#include "../FileStore.h"
#include "../../CompilerCangjieProject.h"

using namespace Cangjie;
using namespace CONSTANTS;
using namespace Cangjie::FileUtil;

namespace {
std::string GetParentPath(const std::string &filePath)
{
    size_t lastSlashPos = filePath.find_last_of("/\\");
    if (lastSlashPos == std::string::npos) {
        return "";
    }
    return filePath.substr(0, lastSlashPos);
}

size_t GetMatchingPathLength(const ark::ModuleInfo &moduleInfo, const std::string &filePath)
{
    size_t matchingLength = 0;
    auto updateMatchingLength = [&filePath, &matchingLength](const std::string &path) {
        if (!path.empty() && ark::IsUnderPath(path, filePath, true)) {
            matchingLength = std::max(matchingLength, path.size());
        }
    };
    updateMatchingLength(moduleInfo.modulePath);
    updateMatchingLength(moduleInfo.srcPath);
    updateMatchingLength(moduleInfo.commonSpecificPaths.first);
    for (const auto &specificPath : moduleInfo.commonSpecificPaths.second) {
        updateMatchingLength(specificPath);
    }
    return matchingLength;
}
}

namespace ark {
void ModuleManager::WorkspaceModeParser(const std::string &workspace)
{
    if (multiModuleOption.is_null()) {
        std::string moduleName = CONSTANTS::DEFAULT_ROOT_PACKAGE();
        std::string normalizeModulePath = FileStore::NormalizePath(URI::Resolve(workspace));
        duplicateModules[moduleName].push_back(normalizeModulePath);
        moduleInfoMap[normalizeModulePath] = {.moduleName = moduleName, .modulePath = normalizeModulePath,
                                              .cjoRequiresMap = {}, .srcPath = {},
                                              .isCommonSpecificModule = false,
                                              .commonSpecificPaths = {}, .sourceSetNames = {},
                                              .sourceSetNameByPath = {}};
        requirePackages[moduleName].insert(moduleName);
        return;
    }
    for (const auto &moduleOptItem : multiModuleOption.items()) {
        auto &key = moduleOptItem.key();
        const nlohmann::json &value = multiModuleOption[key];
        std::string path = FileStore::NormalizePath(URI::Resolve(key));
        std::string name = CONSTANTS::DEFAULT_ROOT_PACKAGE();
        if (value != nullptr && value.contains(MODULE_JSON_NAME())) {
            name = value.value(MODULE_JSON_NAME(), "");
        }
        duplicateModules[name].push_back(path);
        moduleInfoMap[path] = {.moduleName = name, .modulePath = path, .cjoRequiresMap = {}, .srcPath = {},
                               .isCommonSpecificModule = false, .commonSpecificPaths = {}, .sourceSetNames = {},
                               .sourceSetNameByPath = {}};
        if (value.contains(SRC_PATH())) {
            auto srcPath = value.value(SRC_PATH(), "");
            moduleInfoMap[path].srcPath = FileStore::NormalizePath(URI::Resolve(srcPath));
        } else if (value.contains(COMMON_SPECIFIC_PATHS())) {
            SetCommonSpecificPath(value, path);
        }
        if (value.contains(COMBINED())) {
            combinedMap[name] = value.value(COMBINED(), false);
        }
        (void)requirePackages[name].insert(name);

        SetPackageRequires(value, path);

        if (value.contains(REQUIRES())) {
            if (!value[REQUIRES()].is_object()) {
                continue;
            }
            for (const auto &item : value[REQUIRES()].items()) {
                auto &reqKey = item.key();
                const auto &requireOption = value[REQUIRES()][reqKey];
                if (!requireOption.is_object()) {
                    continue;
                }
                auto itemPath = requireOption.value(MODULE_JSON_PATH(), "");
                if (itemPath.empty()) {
                    continue;
                }
                std::string requirePath = FileStore::NormalizePath(URI::Resolve(itemPath));
                if (!FileExist(requirePath)) {
                    continue;
                }
                bool isScriptDependence = requireOption.value(IS_SCRIPT_DEPENDENCE(), false);
                if (isScriptDependence) {
                    (void)scriptRequirePackages[name].insert(reqKey);
                } else {
                    (void)requirePackages[name].insert(reqKey);
                    (void)directRequirePaths[path].insert_or_assign(reqKey, requirePath);
                }
            }
        }
    }
}

void ModuleManager::SetCommonSpecificPath(const nlohmann::json &jsonData, const std::string &modulePath)
{
    if (!jsonData.contains(COMMON_SPECIFIC_PATHS()) || !jsonData[COMMON_SPECIFIC_PATHS()].is_array()) {
        return;
    }
    moduleInfoMap[modulePath].isCommonSpecificModule = true;
    std::vector<std::string> specificSourceSetNames;
    for (const auto &member : jsonData[COMMON_SPECIFIC_PATHS()]) {
        if (!member.is_object() || !member.contains(TYPE()) || !member.contains(PATH())) {
            continue;
        }
        const auto &type = member.value(TYPE(), "");
        const auto &path = member.value(PATH(), "");
        if (path == "") {
            continue;
        }
        const auto normalizedPath = FileStore::NormalizePath(URI::Resolve(path));
        if (type == COMMON() && moduleInfoMap[modulePath].commonSpecificPaths.first == "") {
            moduleInfoMap[modulePath].commonSpecificPaths.first = normalizedPath;
            moduleInfoMap[modulePath].sourceSetNameByPath[normalizedPath] = "common";
            continue;
        }
        if (type == SPECIFIC()) {
            const auto sourceSetName = member.value(SOURCE_SET_NAME(), "");
            if (sourceSetName.empty()) {
                continue;
            }
            moduleInfoMap[modulePath].commonSpecificPaths.second.push_back(normalizedPath);
            moduleInfoMap[modulePath].sourceSetNameByPath[normalizedPath] = sourceSetName;
            specificSourceSetNames.push_back(sourceSetName);
            continue;
        }
    }
    if (!moduleInfoMap[modulePath].commonSpecificPaths.first.empty()) {
        moduleInfoMap[modulePath].sourceSetNames.push_back("common");
    }
    moduleInfoMap[modulePath].sourceSetNames.insert(moduleInfoMap[modulePath].sourceSetNames.end(),
        specificSourceSetNames.begin(), specificSourceSetNames.end());
}

void ModuleManager::SetPackageRequires(const nlohmann::json &jsonData, const std::string &modulePath)
{
    std::string path;
    std::string normalizePath;
    std::string cjoModuleName;
    if (jsonData.contains(PACKAGES_REQUIRES())) {
        if (jsonData[PACKAGES_REQUIRES()].contains(PACKAGE_OPTION())) {
            auto items = jsonData[PACKAGES_REQUIRES()][PACKAGE_OPTION()].items();
            for (const auto &item : items) {
                auto &key = item.key();
                path = jsonData[PACKAGES_REQUIRES()][PACKAGE_OPTION()].value(key, "");
                normalizePath = FileStore::NormalizePath(URI::Resolve(path));
                if (!FileExist(normalizePath)) {
                    continue;
                }
                cjoModuleName = GetDirName(GetDirPath(path));
                (void)moduleInfoMap[modulePath].cjoRequiresMap.emplace(cjoModuleName, normalizePath);
            }
        }
        if (jsonData[PACKAGES_REQUIRES()].contains(PATH_OPTION()) &&
            jsonData[PACKAGES_REQUIRES()][PATH_OPTION()].is_array()) {
            for (auto &member : jsonData[PACKAGES_REQUIRES()][PATH_OPTION()]) {
                std::string cjoDir = FileStore::NormalizePath(URI::Resolve(member.get<std::string>()));
                if (!FileExist(cjoDir)) {
                    continue;
                }
                for (const auto &cjoFileName : GetAllFilesUnderCurrentPath(cjoDir, "cjo")) {
                    path = NormalizePath(JoinPath(cjoDir, cjoFileName));
                    auto cjoPackageName = GetFileNameWithoutExtension(cjoFileName);
                    (void)moduleInfoMap[modulePath].cjoRequiresMap.emplace(cjoPackageName, path);
                }
            }
        }
    }
}

std::unordered_set<std::string> ModuleManager::GetAllRequiresOneModule(
    const std::string &require,
    std::unordered_map<std::string, bool> &isVisited,
    bool includeScriptRequire)
{
    std::unordered_set<std::string> res;
    if (isVisited[require]) {
        return res;
    }
    isVisited[require] = true;
    auto deps = requirePackages[require];
    if (includeScriptRequire && scriptRequirePackages.count(require) != 0) {
        deps.insert(scriptRequirePackages[require].begin(), scriptRequirePackages[require].end());
    }

    if (deps.empty()) {
        return res;
    }
    for (const auto &dependent : deps) {
        auto temp = GetAllRequiresOneModule(dependent, isVisited, includeScriptRequire);
        res.insert(temp.begin(), temp.end());
    }
    res.insert(deps.begin(), deps.end());
    return res;
}

std::unordered_set<std::string> ModuleManager::GetBuildScriptRequiresOneModule(const std::string &moduleName)
{
    std::unordered_set<std::string> res;
    if (moduleName.empty()) {
        return res;
    }
    (void)res.insert(moduleName);
    auto scriptFound = scriptRequirePackages.find(moduleName);
    if (scriptFound == scriptRequirePackages.end()) {
        return res;
    }
    res.insert(scriptFound->second.begin(), scriptFound->second.end());
    for (const auto &dependent : scriptFound->second) {
        std::unordered_map<std::string, bool> isVisited;
        auto temp = GetAllRequiresOneModule(dependent, isVisited, true);
        res.insert(temp.begin(), temp.end());
    }
    return res;
}

void ModuleManager::SetRequireAllPackages()
{
    std::unordered_set<std::string> moduleNames;
    for (const auto &require : requirePackages) {
        moduleNames.insert(require.first);
    }
    for (const auto &require : scriptRequirePackages) {
        moduleNames.insert(require.first);
    }
    for (const auto &moduleName : moduleNames) {
        std::unordered_map<std::string, bool> isVisited;
        auto normalItem = GetAllRequiresOneModule(moduleName, isVisited, false);
        (void)requireAllPackages.emplace(moduleName, normalItem);

        auto buildItem = GetBuildScriptRequiresOneModule(moduleName);
        (void)requireAllPackagesInBuild.emplace(moduleName, buildItem);
    }
}

std::string ModuleManager::GetProjectModuleName() const
{
    auto found = moduleInfoMap.find(projectRootPath);
    if (found == moduleInfoMap.end()) {
        return "";
    }
    return found->second.moduleName;
}

bool ModuleManager::IsBuildScriptFile(const std::string &filePath) const
{
    std::string normalizedFilePath = Normalize(filePath);
    std::string buildScriptPath = Normalize(JoinPath(projectRootPath, BUILD_SCRIPT_FILE_NAME()));
    return normalizedFilePath == buildScriptPath;
}

std::string ModuleManager::GetExpectedPkgName(const Cangjie::AST::File &file)
{
    for (const auto &iter : moduleInfoMap) {
        auto curModulePath = 
        CompilerCangjieProject::GetInstance()->GetModuleSrcPath(iter.second.modulePath, file.filePath);
        if (!IsUnderPath(curModulePath, file.filePath)) {
            continue;
        }
        auto parentDirPath = ::GetParentPath(file.filePath);
        if (curModulePath == parentDirPath) {
            return iter.second.moduleName;
        }
    }
    std::string path = Normalize(file.filePath);
    std::string fullPkgName = CompilerCangjieProject::GetInstance()->GetFullPkgName(path);
    return CompilerCangjieProject::GetInstance()->GetRealPackageName(fullPkgName);
}

bool ModuleManager::GetModuleNameConflict(const std::string &filePath, ModuleNameConflict &conflict) const
{
    const std::string normalizedFilePath = FileStore::NormalizePath(filePath);
    const ModuleInfo *currentModule = nullptr;
    size_t matchingLength = 0;
    for (const auto &item : moduleInfoMap) {
        size_t currentMatchingLength = GetMatchingPathLength(item.second, normalizedFilePath);
        if (currentMatchingLength > matchingLength) {
            matchingLength = currentMatchingLength;
            currentModule = &item.second;
        }
    }
    if (currentModule == nullptr) {
        return false;
    }

    return ResolveModuleNameConflict(*currentModule, conflict);
}

bool ModuleManager::ResolveModuleNameConflict(
    const ModuleInfo &currentModule, ModuleNameConflict &conflict) const
{
    if (currentModule.moduleName.empty() || !IsUnderPath(projectRootPath, currentModule.modulePath, true)) {
        return false;
    }

    auto directRequires = directRequirePaths.find(currentModule.modulePath);
    if (directRequires == directRequirePaths.end()) {
        return false;
    }
    auto sameNameRequire = directRequires->second.find(currentModule.moduleName);
    if (sameNameRequire == directRequires->second.end() || sameNameRequire->second == currentModule.modulePath) {
        return false;
    }

    // Report only a conflict proven by the resolved path dependency graph. Merely seeing two modules with the
    // same name in multiModuleOption is insufficient because they may not belong to the same dependency tree.
    auto requiredModule = moduleInfoMap.find(sameNameRequire->second);
    if (requiredModule == moduleInfoMap.end() || requiredModule->second.moduleName != currentModule.moduleName) {
        return false;
    }

    conflict.moduleName = currentModule.moduleName;
    conflict.modulePaths = {currentModule.modulePath, requiredModule->second.modulePath};
    return true;
}

bool ModuleManager::isCommonSpecificModule(const std::string &filePath)
{
    std::string normalizeFilePath = Normalize(filePath);
    for (const auto &item : moduleInfoMap) {
        if (IsUnderPath(item.second.modulePath, normalizeFilePath, true)) {
            return item.second.isCommonSpecificModule;
        }
    }
    return false;
}
} // namespace ark
