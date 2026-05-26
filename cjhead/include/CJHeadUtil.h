// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJ_HEAD_UTIL_H
#define CJ_HEAD_UTIL_H

#include <regex>
#include <string>
#include <vector>

#include "cangjie/Lex/Lexer.h"
#ifdef USE_CXX17_FEATURES
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

using namespace Cangjie;
using namespace Cangjie::AST;

namespace CJHead {
class CJHeadUtil {
public:
    static auto GetComments(const std::string &p) -> std::vector<Token>;

    static auto SplitStringTolines(const std::string &s) -> std::vector<std::string>;

    static void PrintHelp();

    static auto CheckPath(const std::string &optarg) -> bool;

    static auto ConvertArgv(int argc, char **argv) -> std::vector<std::string>;
};
}  // namespace CJHead

#endif