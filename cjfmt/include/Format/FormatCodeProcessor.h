// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_FORMATCODEPROCESSOR_H
#define CJFMT_FORMATCODEPROCESSOR_H

#include "Format/ASTToFormatSource.h"
#include "Format/Doc.h"
#include "cangjie/Basic/Print.h"
#include "cangjie/Parse/Parser.h"
#include "cangjie/Utils/FileUtil.h"

#include <dirent.h>
#include <fstream>
#include <istream>
#include <map>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <vector>
namespace Cangjie::Format {
constexpr int OK = 0;
constexpr int ERR = 1;
const int DEPTH_OF_RECURSION = 0;
/* Sets the maximum recursion depth.The value -1 indicates that the recursion depth is not limited.It will be used as a
 * configuration option later. */
const int MAX_RECURSION_DEPTH = -1;

int FmtDir(const std::string& fmtDirPath, const std::string& dirOutputPath);
std::optional<std::string> FormatText(const std::string& rawCode, const std::string& filepath, Region regionToFormat);
bool FormatFile(std::string& rawCode, const std::string& filepath, std::string& sourceFormat, Region regionToFormat);
bool HasEnding(std::string const& fullString, std::string const& ending);
void TraveDepthLimitedDirs(
    const std::string& path, std::map<std::string, std::string>& fileMap, int depth, const int maxDepth);
std::string GetTargetName(
    const std::string& file, const std::string& fileName, const std::string& fmtDirPath, const std::string& outPath);
std::string PathJoin(const std::string& baseDir, const std::string& baseName);
} // namespace Cangjie::Format
#endif // CJFMT_FORMATCODEPROCESSOR_H
