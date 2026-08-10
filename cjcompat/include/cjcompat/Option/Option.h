// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef OPTION_OPTION_H
#define OPTION_OPTION_H

#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#ifdef __linux__
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

#include "cangjie/Basic/Print.h"
#include "cangjie/Option/Option.h"

#ifdef __linux__
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

namespace CjComplienceChecker {
/**
 * All Params used in the cjcompat tool, i.e.
 * cjoPathOld: cjo file corresponding to the old version of a Cangjie library.
 * cjoPathNew: cjo file corresponding to the new version of a Cangjie library.
 * cjoModulesPath: path corresponding to cjo location from the Cangjie standard library.
 * enableABI: Enforces rules for ABI compatibility.
 * enableAPI: Enforces rules for API compatibility.
 */
struct Options {
    std::string cjoPathNew{};
    std::string cjoPathOld{};
    std::string cjoModulesPath{};
    std::vector<std::string> cangjiePaths;
    std::vector<std::string> importPaths;
    bool enableABI{false};
    bool enableAPI{false};
#ifndef NDEBUG
    bool printDebugInfo{false};
#endif
};

enum Status {
    OK = 0,
    ERR = 1,
    END = 2,
};

using ArgType = std::variant<bool, const char*>;

static const std::unordered_map<std::string, std::function<Status(CjComplienceChecker::Options&, ArgType)>>
    optionMapArg = {
        {"--old", [](CjComplienceChecker::Options& params, ArgType optarg) -> Status {
            if (!params.cjoPathOld.empty()) {
                return ERR;
            }
            params.cjoPathOld = std::string(std::get<const char*>(optarg));
            return OK;
        }},
        {"--new", [](CjComplienceChecker::Options& params, ArgType optarg) -> Status {
             if (!params.cjoPathNew.empty()) {
                 return ERR;
             }
             params.cjoPathNew = std::string(std::get<const char*>(optarg));
             return OK;
        }},
        {"--import-path", [](CjComplienceChecker::Options& params, ArgType optarg) -> Status {
             params.importPaths.emplace_back(std::string(std::get<const char*>(optarg)));
             return OK;
        }}
    };

static const std::unordered_map<std::string, std::function<void(CjComplienceChecker::Options&, ArgType)>>
    optionMapNoArg = {
        {"--api",
            [](CjComplienceChecker::Options& params, ArgType optarg) { params.enableAPI = std::get<bool>(optarg); }},
        {"--abi",
            [](CjComplienceChecker::Options& params, ArgType optarg) { params.enableABI = std::get<bool>(optarg); }},
#ifndef NDEBUG
        {"--printDebugInfo",
            [](CjComplienceChecker::Options& params, ArgType optarg) {
                params.printDebugInfo = std::get<bool>(optarg);
            }},
#endif
};

int HandleOptions(int argc, char** argv, const char** envp, CjComplienceChecker::Options& params);
} // namespace CjComplienceChecker
#endif // OPTION_OPTION_H
