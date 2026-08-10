// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "cjcompat/Option/Option.h"

using namespace Cangjie;
using namespace CjComplienceChecker;

namespace {

/**
 * print cjcompat commit number
 * print dependent Cangjie version
 */
static void PrintVersion()
{
#ifndef CJCOMPAT_VERSION
    Errorln("Can not obtain cjcompat version");
#else
    Println(std::string("Cangjie Compliance: ") + std::string(CJCOMPAT_VERSION));
#endif

#ifndef CJCOMPAT_CANGJIE_VER
    Errorln("Can not obtain cjc version");
#else
    // CJCOMPAT_CANGJIE_VER comes with "Cangjie Compiler" version
    Println(std::string(CJCOMPAT_CANGJIE_VER));
#endif
}

static void PrintHelp(void)
{
    Println("Usage: ");
    Println("       cjcompat --old /path/to/old.cjo --new /path/to/new.cjo [--api] [--abi] [--import-path /path]");
    Println("Options:");
    Println("   -h                      Show usage");
    Println("                               eg: cjcompat -h");
    Println(
        "   --old <value>           Path to the .cjo file corresponding to the old version of a Cangjie library.");
    Println(
        "   --new <value>           Path to the .cjo file corresponding to the new version of a Cangjie library.");
    Println("                               eg: cjcompat --old filePathOld --new filePathNew");
    Println("   --import-path <value>   Add .cjo search path.");
    Println("   --api                   Enforces rules for API compatibility.");
    Println("   --abi                   Enforces rules for ABI compatibility.");
    Println("                               eg: cjcompat --old filePathOld --new filePathNew --api");
}

static int CJCOMPATProcessFlag(CjComplienceChecker::Options& options)
{
    if (!options.enableABI && !options.enableAPI) {
        options.enableABI = !options.enableABI;
        options.enableAPI = !options.enableAPI;
    }
    std::string suffix = ".cjo";
    if (options.cjoPathOld.empty() ||
        !(options.cjoPathOld.rfind(suffix) == options.cjoPathOld.size() - suffix.size())) {
        Errorln("Essential argument --old wrong or missing!");
        PrintHelp();
        return ERR;
    }
    if (options.cjoPathNew.empty() ||
        !(options.cjoPathNew.rfind(suffix) == options.cjoPathNew.size() - suffix.size())) {
        Errorln("Essential argument --new wrong or missing!");
        PrintHelp();
        return ERR;
    }
    return OK;
}

static std::unordered_map<std::string, std::string> StringifyEnvironmentPointer(const char** envp)
{
    std::unordered_map<std::string, std::string> environmentVars;
    if (!envp) {
        return environmentVars;
    }
    // Read all environment variables
    for (size_t i = 0;; ++i) {
        if (!envp[i]) {
            break;
        }
        std::string item(envp[i]);
        if (auto pos = item.find('='); pos != std::string::npos) {
            (void)environmentVars.emplace(item.substr(0, pos), item.substr(pos + 1));
        };
    }
    return environmentVars;
}

static int CheckCangjieHome(const char** envp)
{
    std::unordered_map<std::string, std::string> environmentVars = StringifyEnvironmentPointer(envp);
    if (environmentVars.find(CANGJIE_HOME) == environmentVars.end()) {
        Errorln("Can not find CANGJIE_HOME, please setup CANGJIE_HOME");
        return ERR;
    }
    auto cangjieHome = FileUtil::GetAbsPath(environmentVars.at(CANGJIE_HOME));
    if (!cangjieHome.has_value()) {
        Errorln("Can not find realpath of CANGJIE_HOME, please setup CANGJIE_HOME");
        return ERR;
    }
    return OK;
}

static int SetCangjieModulesPath(const char** envp, CjComplienceChecker::Options& options)
{
    std::unordered_map<std::string, std::string> environmentVars = StringifyEnvironmentPointer(envp);
    auto cangjieHome = FileUtil::GetAbsPath(environmentVars.at(CANGJIE_HOME));
    std::string highestCjoDir;
#ifdef _WIN32
    highestCjoDir = cangjieHome.value() + "\\modules";
#else
    highestCjoDir = cangjieHome.value() + "/modules/";
#endif

    try {
        // Check if the path exists and is a directory
        if (fs::exists(highestCjoDir) && fs::is_directory(highestCjoDir)) {
            bool found = false;

            // Iterate through the directory
            for (const auto& entry : fs::directory_iterator(highestCjoDir)) {
                if (fs::is_directory(entry.status())) {
                    highestCjoDir = entry.path().string();
                    found = true;
                    break; // Exit the loop after finding the first directory
                }
            }

            if (!found) {
                return ERR;
            }
        } else {
            return ERR;
        }
    } catch (const fs::filesystem_error& e) {
        return ERR;
    }
    options.cjoModulesPath = highestCjoDir;
    const std::string cangjiePath = "CANGJIE_PATH";
    if (environmentVars.find(cangjiePath) != environmentVars.end()) {
        options.cangjiePaths = FileUtil::SplitEnvironmentPaths(environmentVars.at(cangjiePath));
    }
    return OK;
}

static int HandleNoArgOption(std::string arg, CjComplienceChecker::Options& params)
{
    auto handleOption = optionMapNoArg.find(arg);
    if (handleOption == optionMapNoArg.end()) {
        Errorln("Illegal option: ", arg);
        Println("Try: 'cjcompat -h' for more information.");
        return ERR;
    }
    handleOption->second(params, true);
    return OK;
}
}

int CjComplienceChecker::HandleOptions(int argc, char** argv, const char** envp, CjComplienceChecker::Options& params)
{
    if (CheckCangjieHome(envp) != OK) {
        return ERR;
    }

    if (argc == 1) {
        PrintHelp();
        return END;
    }
    int i = 1;
    while (i < argc) {
        std::string arg = argv[i];
        if (arg == "-v") {
            PrintVersion();
            return END;
        }
        if (arg == "-h") {
            PrintHelp();
            return END;
        }
        auto handleOption = optionMapArg.find(arg);
        if (handleOption == optionMapArg.end()) {
            if (HandleNoArgOption(arg, params) != OK) {
                return ERR;
            }
            // Jump to the next option ('1' means option)
            ++i;
            continue;
        }
        if (i + 1 >= argc ||
            !(optionMapArg.find(argv[i + 1]) == optionMapArg.end() &&
                optionMapNoArg.find(argv[i + 1]) == optionMapNoArg.end())) {
            Errorln("Option that requires an argument: ", arg);
            return ERR;
        }
        if (handleOption->second(params, argv[i + 1]) != OK) {
            Errorln("Argument: ", arg, ", used more than once!");
            return ERR;
        }
        // Jump to the next option ('2' means option and argument)
        i += 2;
    }

    if (CJCOMPATProcessFlag(params) != OK) {
        return ERR;
    }
    SetCangjieModulesPath(envp, params);
    return OK;
}
