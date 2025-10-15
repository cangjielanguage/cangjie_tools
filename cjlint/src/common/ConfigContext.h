// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIECODECHECK_CONFIGCONTEXT_H
#define CANGJIECODECHECK_CONFIGCONTEXT_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "ExcludeRule.h"

namespace Cangjie::CodeCheck {
static const std::string DEFAULT_EXCLUSION_FILE = "cjlint_file_exclude.cfg";
static const std::string DEFAULT_EXCLUSION_FORMAT = ".cfg";

#ifdef _WIN32
const std::string PATH_SEPARATOR = "\\";
#else
const std::string PATH_SEPARATOR = "/";
#endif

class ConfigContext {
public:
    const std::vector<std::string>& GetSrcFileDir() const;
    const std::string& GetConfigFileDir() const;
    const std::string& GetModulesDir() const;
    const std::vector<ExcludeRule>& GetExcludeList() const;
    const std::string& GetReportFile() const;
    const std::string& GetReportFormat() const;
    const std::string& GetCangjieHome() const;
    const std::string GetCangjieHomePath() const;
    std::string GetCjlintConfigPath();
    const std::vector<std::string>& GetSrcFileList() const;

    void SetSrcFileList(const std::vector<std::string>& srcFileList);
    void SetSrcFileDir(size_t index, std::string srcFile);
    void AddSrcFileDir(const std::string srcFileDirInput);
    void SetConfigFileDir(const std::string configFileDirInput);
    void SetModulesDir(const std::string moduleFileDirInput);
    void AddExcludeList(const std::string excludeFile);
    void SetReportFile(const std::string reportFileInput);
    bool SetReportFormat(const std::string reportFormatInput);
    void SetCangjieHome(const std::string cangjieHomePath);
    static ConfigContext &GetInstance()
    {
        static ConfigContext instance;
        return instance;
    }
    void FilterCompileList(const std::vector<std::string> &fileList);
    void CheckDefaultExclusion();
    const std::string& GetCjoPath() const;
    void SetCjoPath(const std::string cjoPathStr);

private:
    ConfigContext() {}
    ~ConfigContext() = default;
    ConfigContext(const ConfigContext&);
    ConfigContext& operator=(const ConfigContext&);
    void AddExcludeRule(const std::string &excludeDir, const std::string &exclude);

#ifdef _WIN32
    const std::string configPath = "\\tools\\";
#else
    const std::string configPath = "/tools/";
#endif
    std::vector<std::string> srcFileDir;
    std::string configFileDir;
    std::string modulesDir;
    std::vector<ExcludeRule> excludeList;
    std::string reportFile;
    std::string reportFormat = "json";
    std::vector<std::string> srcFileList;
    std::string cangjieHome;
    bool defaultExclusion = true;
    std::string cjoPath;
};
}
#endif // CANGJIECODECHECK_CONFIGCONTEXT_H
