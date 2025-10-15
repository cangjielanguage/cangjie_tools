// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Cjlint.h"
#include <common/CommonFunc.h>
#include <regex>
#include <string>
#include <vector>
#include "cangjie/Lex/Lexer.h"
#include "checker/Checker.h"

using namespace Cangjie;
using namespace Cangjie::CodeCheck;
using Json = nlohmann::json;

namespace CjLint {
#ifdef _WIN32
constexpr const char* MODULES_BACKEND_PATH = "/modules/windows_x86_64_cjnative/";
#else
#ifdef __APPLE__
#ifdef __aarch64__
constexpr const char* MODULES_BACKEND_PATH = "/modules/darwin_aarch64_cjnative/";
#else
constexpr const char* MODULES_BACKEND_PATH = "/modules/darwin_x86_64_cjnative/";
#endif
#else
#ifdef __ARM__
constexpr const char* MODULES_BACKEND_PATH = "/modules/linux_aarch64_cjnative/";
#else
constexpr const char* MODULES_BACKEND_PATH = "/modules/linux_x86_64_cjnative/";
#endif
#endif
#endif

constexpr const char* JSONPATH = "/config/exclude_lists.json";
std::vector<CjlintIgnoreInfo> ignoreInfos;
std::unordered_map<std::string, std::string> StringifyEnvironmentPointer(const char** envp)
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

/**
 * print cjlint commit number
 * print dependent Cangjie version
 */
void PrintVersion()
{
#ifndef CJLINT_VERSION
    Errorln("Can not obtain cjlint version");
#else
    Println(std::string("Cangjie Lint: ") + std::string(CJLINT_VERSION));
#endif

#ifndef CJC_VERSION
    Errorln("Can not obtain cjc version");
#else
    // CJC_VERSION comes with "Cangjie Compiler" version
    Println(CJC_VERSION);
#endif
}

void PrintHelp(void)
{
    Println("Usage: ");
    Println("       ./cjlint -f fileDir [option] fileDir...");
    Println("Options:");
    Println("   -h                      Show usage");
    Println("                               eg: ./cjlint -h");
    Println("   -v                      Show version");
    Println("                               eg: ./cjlint -v");
    Println("   -f <value>              Detected file directory, it can be absolute path or relative path");
    Println("                               eg: ./cjlint -f fileDir -c . -m .");
    Println("   -e <v1:v2:...>          Excluded files, directories or configurations, splitted by ':'. "
            "Regular expressions are supported");
    Println("                               eg: ./cjlint -f fileDir -e fileDir/a/:fileDir/b/*.cj");
    Println("   -o <value>              Output file path, it can be absolute path or relative path, "
            "if it is directory, default file name is cjReport");
    Println("                               eg: ./cjlint -f fileDir -o ./out");
    Println("   -r [csv|json]           Report file format, it can be csv or json, default is json");
    Println("                               eg: ./cjlint -f fileDir -r csv -o ./out");
    Println("   -c <value>              Directory path where the config directory is located, it can be absolute path "
            "or relative path to the executable file");
    Println("                               eg: ./cjlint -f fileDir -c .");
    Println("   -m <value>              Directory path where the modules directory is located, it can be absolute path "
            "or relative path to the executable file");
    Println("                               eg: ./cjlint -f fileDir -m .");
    Println("   --import-path <value>   Add .cjo search path");
}

static void GetCJFilePath(const std::string& path, std::vector<std::string>& cjPath)
{
    auto files = FileUtil::GetAllFilesUnderCurrentPath(path, "cj", false);
    auto dirs = FileUtil::GetAllDirsUnderCurrentPath(path);
    for (auto& f : files) {
        cjPath.emplace_back(FileUtil::JoinPath(path, f));
    }
    for (auto& d : dirs) {
        auto subFiles = FileUtil::GetAllFilesUnderCurrentPath(d, "cj", false);
        for (auto& sf : subFiles) {
            cjPath.emplace_back(FileUtil::JoinPath(d, sf));
        }
    }
}

static void SplitLine(const std::string& comment, const std::string& path, int linePos)
{
    auto lineSplit = Utils::SplitLines(comment);
    for (auto& line : lineSplit) {
        std::smatch sm;
        std::regex cjlintIgnoreRegex("cjlint-ignore(\\s+-start|\\s+-end)?(\\s+(!([^\\s(*/)])*))*|"
                                     "cjlint-ignore\\s+(-start|-end)");
        std::regex cjlintRules("!([A-Za-z\\.]+)[0-9]+");
        if (!std::regex_search(line, sm, cjlintIgnoreRegex)) {
            continue;
        }
        CjlintIgnoreInfo cjlintIgnoreInfo;
        cjlintIgnoreInfo.path = path;
        cjlintIgnoreInfo.pos = linePos;
        auto stringSplit = Utils::SplitString(sm.str(), " ");
        bool multiPosSetFlag = true;
        for (auto& str : stringSplit) {
            if (str == "-start" && multiPosSetFlag) {
                cjlintIgnoreInfo.start = linePos;
                multiPosSetFlag = false;
            } else if (str == "-end" && multiPosSetFlag) {
                cjlintIgnoreInfo.end = linePos;
                multiPosSetFlag = false;
            } else if (std::regex_match(str, cjlintRules)) {
                cjlintIgnoreInfo.rules.emplace_back(str.substr(1));
            }
        }
        ignoreInfos.emplace_back(cjlintIgnoreInfo);
        break;
    }
}

static void SplitComments(const std::vector<Token>& comments, const std::string& path)
{
    for (auto& c : comments) {
        SplitLine(c.Value(), path, c.Begin().line);
    }
}

static void GetComments(std::vector<std::string>& cjPaths)
{
    DiagnosticEngine diag;
    SourceManager sm;
    for (auto& p : cjPaths) {
        std::ifstream instream(p);
        std::string buffer((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
        instream.close();
        auto rawCode = buffer;
        auto fileID = sm.AddSource(p, rawCode);
        auto lexer = Lexer(fileID, rawCode, diag, sm);
        (void)lexer.GetTokens();
        auto comments = lexer.GetComments();
        SplitComments(comments, p);
    }
}

static void AnalyseComments(
    std::stack<CjlintIgnoreInfo>& multiCjlintIgnore, std::vector<CjlintIgnoreInfo>& finalCjlintIgoreInfos)
{
    for (auto& item : ignoreInfos) {
        if (item.start != 0) {
            multiCjlintIgnore.push(item);
            continue;
        }
        if (item.end != 0 && !multiCjlintIgnore.empty()) {
            auto info = multiCjlintIgnore.top();
            if (info.path == item.path && info.start < item.end) {
                info.end = item.end;
                finalCjlintIgoreInfos.emplace_back(info);
                multiCjlintIgnore.pop();
            }
            continue;
        }
        finalCjlintIgoreInfos.emplace_back(item);
    }
}

static int CheckCode()
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    auto srcFileDirs = configContext.GetSrcFileDir();
    auto reportFile = configContext.GetReportFile();
    auto format = configContext.GetReportFormat();
    SourceManager sm;
    CodeCheckDiagnosticEngine diagEngine;
    diagEngine.SetSourceManager(&sm);
    Json jsonInfo;
    (void)CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, configContext, jsonInfo);
    diagEngine.SetJsonInfo(jsonInfo);
    std::vector<std::string> cjPaths;
    for (const auto& srcFileDir : srcFileDirs) {
        std::vector<std::string> cjPathsTemp;
        GetCJFilePath(srcFileDir, cjPathsTemp);
        configContext.FilterCompileList(cjPathsTemp);
        cjPathsTemp = configContext.GetSrcFileList();
        cjPaths.insert(cjPaths.end(), cjPathsTemp.begin(), cjPathsTemp.end());
    }
    if (cjPaths.empty()) {
        Errorln("There is no cangjie file or all cangjie files are excluded, "
                "please check file directory and exclude rules");
        return ERR;
    }
    configContext.SetSrcFileList(cjPaths);
    GetComments(cjPaths);
    std::stack<CjlintIgnoreInfo> multiCjlintIgnore;
    std::vector<CjlintIgnoreInfo> finalCjlintIgoreInfos;
    AnalyseComments(multiCjlintIgnore, finalCjlintIgoreInfos);
    diagEngine.SetCjlintIgnoreInfos(finalCjlintIgoreInfos);

    if (!reportFile.empty()) {
        diagEngine.SetReportToFile(reportFile, format);
    }
    std::string path = configContext.GetModulesDir();
    if (path.empty()) {
        path = configContext.GetCangjieHomePath();
        configContext.SetModulesDir(path);
    }
    if (access((path + MODULES_BACKEND_PATH).c_str(), R_OK) != 0) {
        Errorln("Can not find modules, please setup CANGJIE_HOME or use -m option to setup modules path");
        return ERR;
    }

    auto checker = Checker(&diagEngine);
    auto res = checker.CheckCode();
    return res;
}

static int CheckAndSetCangjieHome(const char** envp)
{
    ConfigContext& configContext = ConfigContext::GetInstance();
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
    configContext.SetCangjieHome(cangjieHome.value());
    return OK;
}

// Check if context initialization is correct.
static int CheckInitialContext()
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    // Check src directory
    auto srcFileDirs = configContext.GetSrcFileDir();
    for (size_t index = 0; index < srcFileDirs.size(); index++) {
        auto srcFileDir = configContext.GetSrcFileDir()[index];
        if (srcFileDir.empty()) {
            Errorf("Action has no specific file to operate on.\n");
            CjLint::PrintHelp();
            return ERR;
        }
        auto absFileDir = FileUtil::GetAbsPath(srcFileDir);
        if (absFileDir.has_value()) {
            configContext.SetSrcFileDir(index, absFileDir.value());
        }
    }
    // Check report file
    auto reportFile = configContext.GetReportFile();
    auto configFileDir = configContext.GetConfigFileDir();
    auto modulesDir = configContext.GetModulesDir();
    if (!reportFile.empty()) {
        if (reportFile == ".") {
            (void)reportFile.append("/cjReport");
        }
        if (reportFile.substr(reportFile.length() - 1, 1) == PATH_SEPARATOR) {
            (void)reportFile.append("cjReport");
        }
    }
    // Set report, config and module.
    std::optional<std::string> absReportFile, absConfigFileDir, absModulesDir;
    CommonFunc::getAbsPath(reportFile, absReportFile);
    configContext.SetReportFile(absReportFile.value());
    CommonFunc::getAbsPath(configFileDir, absConfigFileDir);
    configContext.SetConfigFileDir(absConfigFileDir.value());
    CommonFunc::getAbsPath(modulesDir, absModulesDir);
    configContext.SetModulesDir(absModulesDir.value());
    return OK;
}

// Check exclude rules
static int CheckExcludeContext(std::vector<std::string>& excludeInputList)
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    for (auto& excludeInput : excludeInputList) {
        auto excludeFileList = Utils::SplitString(excludeInput, " ");
        for (auto& excludeFile : excludeFileList) {
            configContext.AddExcludeList(excludeFile);
        }
    }
    configContext.CheckDefaultExclusion();

    return OK;
}

static int CJLintHelper(const ParamsInCJLint& strsInCJLint)
{
    ConfigContext& configContext = ConfigContext::GetInstance();
    std::vector<std::string> excludeInputList;
    std::vector<std::string> cjoPathList;
    if (!strsInCJLint.reportFile.empty()) {
        configContext.SetReportFile(strsInCJLint.reportFile);
    }
    if (!strsInCJLint.reportFormat.empty() && !configContext.SetReportFormat(strsInCJLint.reportFormat)) {
        Errorln(strsInCJLint.reportFormat, " is not a supported report file format.");
        return ERR;
    }
    if (!strsInCJLint.configFileDir.empty()) {
        configContext.SetConfigFileDir(strsInCJLint.configFileDir);
    }
    if (!strsInCJLint.srcFileDir.empty()) {
        configContext.AddSrcFileDir(strsInCJLint.srcFileDir);
    }
    if (!strsInCJLint.modulesDir.empty()) {
        configContext.SetModulesDir(strsInCJLint.modulesDir);
    }
    if (!strsInCJLint.excludeRule.empty()) {
        excludeInputList.emplace_back(strsInCJLint.excludeRule);
    }
    if (!strsInCJLint.cjoPath.empty()) {
        configContext.SetCjoPath(strsInCJLint.cjoPath);
        excludeInputList.emplace_back(strsInCJLint.cjoPath);
    }
    if ((CheckInitialContext() != OK) || (CheckExcludeContext(excludeInputList) != OK)) {
        return ERR;
    }
    return CheckCode();
}

int CJLint(const ParamsInCJLint& strsInCJLint, const char** envp)
{
    if (CheckAndSetCangjieHome(envp) != OK) {
        return ERR;
    }
    return CJLintHelper(strsInCJLint);
}
} // namespace CjLint