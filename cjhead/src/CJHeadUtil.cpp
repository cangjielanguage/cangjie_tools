// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "CJHeadUtil.h"
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <iterator>
#include <fstream>
#include <iostream>
#include <regex>
#include "cangjie/Lex/Lexer.h"

using namespace Cangjie;
using namespace Cangjie::AST;

namespace CJHead {
auto CJHeadUtil::GetComments(const std::string &p) -> std::vector<Token>
{
    DiagnosticEngine diag;
    SourceManager sm;
    std::ifstream instream(p);
    std::string buffer((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    instream.close();
    auto rawCode = buffer;
    auto fileID = sm.AddSource(p, rawCode);
    auto lexer = Lexer(fileID, rawCode, diag, sm);
    (void)lexer.GetTokens();
    return lexer.GetComments();
}

auto CJHeadUtil::SplitStringTolines(const std::string &s) -> std::vector<std::string>
{
    if (s.empty()) {
        return {};
    }
    std::vector<std::string> lines;
    std::istringstream s_str(s);
    std::string line;
    auto ltrim = [](const std::string &s) { return std::regex_replace(s, std::regex("^\\s+"), ""); };
    while (std::getline(s_str, line)) {
        std::string line_remove_space = ltrim(line);
        lines.emplace_back(line_remove_space);
    }
    return lines;
}

auto CJHeadUtil::CheckPath(const std::string &optarg) -> bool
{
    fs::path p{optarg};
    if (!fs::exists(p)) {
        std::cerr << "Invalid argument value"
                  << "\n";
        exit(-1);
    }
    return true;
}

auto CJHeadUtil::ConvertArgv(int argc, char **argv) -> std::vector<std::string>
{
    std::vector<std::string> result;
    for (int i = 1; i < argc; ++i) {
        result.emplace_back(argv[i]);
    }
    return result;
}

void CJHeadUtil::PrintHelp()
{
    std::cout << "Usage:"
                 "  cjhead [OPTION...]\n\n"
                 "OPTION:\n"
                 "  -i,              Input directory\n"
                 "  -o,              Output directory(default: .\\)\n"
                 "  -p,              Enable package mode, save all content in one package to one file\n"
                 "  --import-path,   Cjo path to scan\n"
                 "  --macro-path,    Macro path to resolve the macro\n"
                 "  --exclude,       Config file describe files not to scan\n"
                 "  -h,              Print help\n";
}
}  // namespace CJHead
