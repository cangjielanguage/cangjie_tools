// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CANGJIE_CODECHECK_CJLINT_H
#define CANGJIE_CODECHECK_CJLINT_H

#include <string>

namespace CjLint {
/**
 * All Params used in the lint tool, i.e.
 * srcFileDir: Detected file directory, it can be absolute path or relative path,
 *             if it is directory, default file name is cjReport
 * modulesDir: Directory path where the modules directory is located,
 *             it can be absolute path or relative path to the executable file
 * excludeRule: Excluded files, directories or configurations, splitted by ':'.
 *              Regular expressions are supported
 * configFileDir: Directory path where the config directory is located,
 *                it can be absolute path or relative path to the
 * executable file reportFormat: Report file format, it can be csv or json, default is json
 * reportFile: Output file path, it can be absolute path or relative path
 */
struct ParamsInCJLint {
    std::string srcFileDir;
    std::string modulesDir;
    std::string excludeRule;
    std::string configFileDir;
    std::string reportFormat;
    std::string reportFile;
    std::string cjoPath;
};

void PrintHelp(void);
void PrintVersion(void);
int CJLint(const ParamsInCJLint &strsInCJLint);
int CJLint(const ParamsInCJLint& strsInCJLint, const char** envp);
}
#endif