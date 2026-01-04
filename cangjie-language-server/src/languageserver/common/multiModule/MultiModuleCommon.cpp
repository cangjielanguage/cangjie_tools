// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "MultiModuleCommon.h"
#include "cangjie/Utils/FileUtil.h"
#include "../../logger/Logger.h"
#include "../Utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

using namespace Cangjie;
using namespace CONSTANTS;
using namespace Cangjie::FileUtil;

namespace {
bool startsWith(const std::string& str, const std::string prefix)
{
    return (str.rfind(prefix, 0) == 0);
}
}

namespace ark {
#ifdef _WIN32
const std::string MACRO_DETAIL = "dll";
#elif __APPLE__
const std::string MACRO_DETAIL = "dylib";
#else
const std::string MACRO_DETAIL = "so";
#endif
const std::unordered_set<std::string> IGNORE_DYNAMIC = {"libstdx.fuzz.fuzz"};

std::string GetLSPServerDir()
{
    char path[PATH_MAX] = {0};
#ifdef _WIN32
    if (GetModuleFileName(nullptr, path, sizeof(path)) == 0) {
        return "";
    }
#else
    if (readlink("/proc/self/exe", path, sizeof(path)) == 0) {
        return "";
    }
#endif
    std::string dirPath(path);
    return GetDirPath(dirPath);
}

std::string GetFileContents(const std::string &fileName)
{
    std::string failedReason;
    auto contents = ReadFileContent(fileName, failedReason);
    if (!failedReason.empty()) {
        Logger::Instance().LogMessage(MessageType::MSG_WARNING,
                                      "error to read form file:" + fileName + " reason:" + failedReason);
        // file is empty, buffer is nullptr, not need delete
        return "";
    }
    if (!contents) {
        return "";
    }
    return *contents;
}

void GetMacroLibPath(const std::string &targetLib,
                     const std::unordered_map<std::string, ModuleInfo> &moduleInfoMap,
                     std::vector<std::string> &macroLibs)
{
    std::string soPath;
    std::string fullName;
    std::unordered_set<std::string> soRecordMap;
    std::unordered_set<std::string> folderRecordMap;
    for (const auto &item : moduleInfoMap) {
        const std::string soFolderPath = JoinPath(targetLib, item.second.moduleName);
        if (!FileExist(soFolderPath) || folderRecordMap.find(soFolderPath) != folderRecordMap.end()) {
            continue;
        }
        for (const auto &file : GetAllFilesUnderCurrentPath(soFolderPath, MACRO_DETAIL)) {
            soPath = JoinPath(soFolderPath, file);
            fullName = LSPJoinPath(GetDirName(soFolderPath), file);
            macroLibs.emplace_back(soPath);
            (void)soRecordMap.emplace(fullName);
        }
        (void)folderRecordMap.emplace(soFolderPath);
    }
    for (const auto &item : moduleInfoMap) {
        for (auto &path : item.second.cjoRequiresMap) {
            const std::string soDirPath = GetDirPath(path.second);
            // LCOV_EXCL_START
            if (!FileExist(soDirPath) || folderRecordMap.find(soDirPath) != folderRecordMap.end()) {
                continue;
            }
            for (auto &file : GetAllFilesUnderCurrentPath(soDirPath, MACRO_DETAIL)) {
                if (!startsWith(file, "lib-macro_")) {
                    continue;
                }
                soPath = JoinPath(soDirPath, file);
                fullName = LSPJoinPath(GetDirName(soDirPath), file);
                std::string dynamicName = GetFileNameWithoutExtension(soPath);
                if (soRecordMap.find(fullName) == soRecordMap.end() &&
                    IGNORE_DYNAMIC.find(dynamicName) == IGNORE_DYNAMIC.end()) {
                    macroLibs.emplace_back(soPath);
                    (void)soRecordMap.emplace(fullName);
                }
            }
            (void)folderRecordMap.emplace(soDirPath);
            // LCOV_EXCL_STOP
        }
    }
} // namespace ark
 
std::string GetCjcPath(const std::string &oldPath)
{
    std::string path = ark::PathWindowsToLinux(Normalize(oldPath));
    std::regex pathRegex("/runtime/lib");
    std::vector<std::string> vectorString(std::sregex_token_iterator(path.begin(), path.end(), pathRegex, -1),
        std::sregex_token_iterator());
    path = vectorString[0];
    std::string cjcPath;
    const std::string cjcBinPath = "bin/";
    cjcPath = JoinPath(path, cjcBinPath);
    Logger::Instance().LogMessage(MessageType::MSG_INFO, "cjcPath is " + cjcPath);
    return cjcPath;
}

// modulesHomeOption > LSPServerDir > environment
std::string GetModulesHome(const std::string &modulesHomeOption, const std::string &environmentHome)
{
    Logger &logger = Logger::Instance();
    std::stringstream log;
    std::string modules = JoinPath(modulesHomeOption, "modules");
    if (!modulesHomeOption.empty() && FileExist(modules)) {
        CleanAndLog(log, "Load modules from (modulesHomeOption):" + modulesHomeOption);
        logger.LogMessage(MessageType::MSG_INFO, log.str());
        return modulesHomeOption;
    }
    std::string lspServerHome = GetLSPServerDir();
    modules = JoinPath(lspServerHome, "modules");
    if (!lspServerHome.empty() && FileExist(modules)) {
        CleanAndLog(log, "Load modules from (LSPServerDir):" + lspServerHome);
        logger.LogMessage(MessageType::MSG_INFO, log.str());
        return lspServerHome;
    }
    modules = JoinPath(environmentHome, "modules");
    if (!environmentHome.empty() && FileExist(modules)) {
        CleanAndLog(log, "Load modules from (environment):" + environmentHome);
        logger.LogMessage(MessageType::MSG_INFO, log.str());
        return environmentHome;
    }
    CleanAndLog(log, "Load modules fail");
    logger.LogMessage(MessageType::MSG_ERROR, log.str());
    return "";
}
}