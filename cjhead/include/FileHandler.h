// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJHEAD_FILE_HANDLER_H
#define CJHEAD_FILE_HANDLER_H

#include <exception>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <regex>

#include "CJHeadCompilerInstance.h"
#include "CJHeadParam.h"
#include "CJHeadUtil.h"
#include "HeadFileVisitor.h"

#include "cangjie/AST/Node.h"
#include "cangjie/Lex/Lexer.h"
#include "cangjie/Utils/FileUtil.h"
#include "cangjie/Utils/Utils.h"
#ifdef USE_CXX17_FEATURES
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace CJHead {
class FileHandler {
public:
    using HeadFileFormat = HeadFileVisitor::HeadFileFormat;

    FileHandler(
        int argc, const std::vector<std::string> &argv, const std::unordered_map<std::string, std::string> &env);

    auto ParseArguments() -> bool;

    auto Run() -> bool;

    auto TraverseAllFiles(const fs::path &input_dir) -> bool;

private:
    int argc_;

    std::vector<std::string> argv_;

    std::unordered_map<std::string, std::string> environment_vars_;

    fs::path input_dir_;

    fs::path output_dir_;

    bool package_mode_;

    std::string input_file_;

    std::string import_path_;

    std::string macro_path_;

    fs::path exclude_config_;

    std::vector<std::regex> exclude_regs_;

    std::string full_package_name_;

    /* head file content */
    HeadFileFormat header_file_;

    std::string first_comment_;

    std::vector<std::string> import_info_;
    /*- head file content */

    auto ParseExcludeConfig() -> bool;
    auto HandlePackage(const fs::path &input_dir) -> bool;
    auto IsExcludeFile(const std::string &file_name) -> bool;

    auto HandleSingleFile(const std::unique_ptr<CJHeadCompilerInstance> &compiler_instance,
        const OwnedPtr<File> &cur_file) -> std::string;

    auto Output(const fs::path &input_dir, const std::string &formatted_content) -> bool;

    static const std::unordered_map<std::string, std::function<void(CJHeadParam &, const std::string &)>> option_map_;
};
}  // namespace CJHead

#endif