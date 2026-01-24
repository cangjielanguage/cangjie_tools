// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_MULTIMODULECOMMON_H
#define LSPSERVER_MULTIMODULECOMMON_H

#include <string>
#include <sstream>
#include <unordered_map>
#include <regex>

namespace ark {
enum class CangjieFileKind {
    MISSING,
    IN_OLD_PACKAGE,
    IN_NEW_PACKAGE,
    IN_PROJECT_NOT_IN_SOURCE
};

struct ModuleInfo {
    std::string moduleName;
    std::string modulePath;
    std::unordered_map<std::string, std::string> cjoRequiresMap;
    std::string srcPath;
    bool isCommonSpecificModule = false;
    // common root path -> specific root path
    std::pair<std::string, std::vector<std::string>> commonSpecificPaths;
    std::vector<std::string> sourceSetNames;
};

std::string GetLSPServerDir();

std::string GetFileContents(const std::string &fileName);

void GetMacroLibPath(const std::string &targetLib,
                     const std::unordered_map<std::string, ModuleInfo> &moduleInfoMap,
                     std::vector<std::string> &macroLibs);
 
std::string GetCjcPath(const std::string &oldPath);

std::string GetModulesHome(const std::string &modulesHomeOption, const std::string &environmentHome);
} // namespace ark

#endif // LSPSERVER_MULTIMODULECOMMON_H
