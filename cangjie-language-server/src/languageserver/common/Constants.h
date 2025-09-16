// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_CONSTANTS_H
#define LSPSERVER_CONSTANTS_H

#include <set>
namespace CONSTANTS {
#ifdef __WIN32
    const std::string BLANK = "\r\n";
#else
    const std::string STRING_BLANK = "\\n";
    const std::string BLANK = "\n";
#endif
    const std::string BUILD_FOLDER_RELEASE = "target/release";
    const std::string BUILD_FOLDER_DEBUG = "target/debug";
    const std::string TARGET_LIB = "targetLib";
    const std::string SOURCE_CODE_DIR = "src";
    const std::string MULTI_MODULE_OPTION = "multiModuleOption";
    const std::string CACHE_PATH = "cachePath";
    const std::string CONDITION_COMPILE_OPTION = "conditionCompileOption";
    const std::string MODULE_CONDITION_COMPILE_OPTION = "moduleConditionCompileOption";
    const std::string SINGLE_CONDITION_COMPILE_OPTION = "singleConditionCompileOption";
    const std::string CONDITION_COMPILE_PATHS = "conditionCompilePaths";
    const std::string MODULES_HOME_OPTION = "modulesHomeOption";
    const std::string STD_LIB_PATH_OPTION = "stdLibPathOption";
    const std::string STD_CJD_PATH_OPTION = "stdCjdPathOption";
    const std::string OHOS_CJD_PATH_OPTION = "ohosCjdPathOption";
    const std::string CJD_CACHE_PATH_OPTION = "cjdCachePathOption";
    const std::string SLASH = "/";
    const std::string DOT = ".";
    const std::string FILE_SEPARATOR = "/\\";
    // key of module.json
    const std::string MODULE_JSON_NAME = "name";
    const std::string REQUIRES = "requires";
    const std::string MODULE_JSON_PATH = "path";
    const std::string PACKAGES_REQUIRES = "package_requires";
    const std::string PACKAGE_OPTION = "package_option";
    const std::string PATH_OPTION = "path_option";
    const std::string DEFAULT_ROOT_PACKAGE = "default";
    const std::string SRC_PATH = "src_path";
    const std::set<std::string> BUILTIN_OPERATORS = {"@", ".", "[]", "()", "++", "--", "?", "!", "-", "**", "*", "/",
                                                     "%", "+", "<<", ">>", "<", "<=", ">", ">=", "is", "as", "==",
                                                     "!=", "&", "^", "|", "..", "..=", "&&", "||", "\?\?",
                                                     "~>", "=", "**=", "*=", "/=", "%/", "+=", "-=", "<<=", ">>=",
                                                     "&=", "^=", "|="};
    // cangjie file extension
    const std::string CANGJIE_FILE_EXTENSION = "cj";
    // cjc auto create file about macro expand
    const std::string CANGJIE_MACRO_FILE_EXTENSION = "macrocall";
    const unsigned int AD_OFFSET = 2;
    constexpr size_t MAC_THREAD_STACK_SIZE = 1024 * 1024 * 8;
    const unsigned int SORT_TEXT_SIZE = 6;
    const float ROUND_NUM = 0.5;
} // namespace CONSTANTS

#endif // LSPSERVER_CONSTANTS_H
