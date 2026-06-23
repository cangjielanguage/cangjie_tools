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
    inline const std::string& BLANK()
    {
        static const std::string value = "\r\n";
        return value;
    }
#else
    inline const std::string& STRING_BLANK()
    {
        static const std::string value = "\\n";
        return value;
    }
    inline const std::string& BLANK()
    {
        static const std::string value = "\n";
        return value;
    }
#endif
    inline const std::string& BUILD_FOLDER_RELEASE()
    {
        static const std::string value = "target/release";
        return value;
    }
    inline const std::string& BUILD_FOLDER_DEBUG()
    {
        static const std::string value = "target/debug";
        return value;
    }
    inline const std::string& TARGET_LIB()
    {
        static const std::string value = "targetLib";
        return value;
    }
    inline const std::string& SOURCE_CODE_DIR()
    {
        static const std::string value = "src";
        return value;
    }
    inline const std::string& MULTI_MODULE_OPTION()
    {
        static const std::string value = "multiModuleOption";
        return value;
    }
    inline const std::string& CACHE_PATH()
    {
        static const std::string value = "cachePath";
        return value;
    }
    inline const std::string& CONDITION_COMPILE_OPTION()
    {
        static const std::string value = "conditionCompileOption";
        return value;
    }
    inline const std::string& MODULE_CONDITION_COMPILE_OPTION()
    {
        static const std::string value = "moduleConditionCompileOption";
        return value;
    }
    inline const std::string& SINGLE_CONDITION_COMPILE_OPTION()
    {
        static const std::string value = "singleConditionCompileOption";
        return value;
    }
    inline const std::string& CONDITION_COMPILE_PATHS()
    {
        static const std::string value = "conditionCompilePaths";
        return value;
    }
    inline const std::string& MODULES_HOME_OPTION()
    {
        static const std::string value = "modulesHomeOption";
        return value;
    }
    inline const std::string& SINGLE_FILE_PATH_OPTION()
    {
        static const std::string value = "singleFilePath";
        return value;
    }
    inline const std::string& STD_LIB_PATH_OPTION()
    {
        static const std::string value = "stdLibPathOption";
        return value;
    }
    inline const std::string& STD_CJD_PATH_OPTION()
    {
        static const std::string value = "stdCjdPathOption";
        return value;
    }
    inline const std::string& OHOS_CJD_PATH_OPTION()
    {
        static const std::string value = "ohosCjdPathOption";
        return value;
    }
    inline const std::string& CJD_CACHE_PATH_OPTION()
    {
        static const std::string value = "cjdCachePathOption";
        return value;
    }
    inline const std::string& SLASH()
    {
        static const std::string value = "/";
        return value;
    }
    inline const std::string& DOT()
    {
        static const std::string value = ".";
        return value;
    }
    inline const std::string& FILE_SEPARATOR()
    {
        static const std::string value = "/\\";
        return value;
    }
    inline const std::string& MODULE_JSON_NAME()
    {
        static const std::string value = "name";
        return value;
    }
    inline const std::string& REQUIRES()
    {
        static const std::string value = "requires";
        return value;
    }
    inline const std::string& MODULE_JSON_PATH()
    {
        static const std::string value = "path";
        return value;
    }
    inline const std::string& IS_SCRIPT_DEPENDENCE()
    {
        static const std::string value = "isScriptDependence";
        return value;
    }
    inline const std::string& PACKAGES_REQUIRES()
    {
        static const std::string value = "package_requires";
        return value;
    }
    inline const std::string& PACKAGE_OPTION()
    {
        static const std::string value = "package_option";
        return value;
    }
    inline const std::string& PATH_OPTION()
    {
        static const std::string value = "path_option";
        return value;
    }
    inline const std::string& DEFAULT_ROOT_PACKAGE()
    {
        static const std::string value = "default";
        return value;
    }
    inline const std::string& SRC_PATH()
    {
        static const std::string value = "src_path";
        return value;
    }
    inline const std::string& SOURCE_SET_NAME()
    {
        static const std::string value = "name";
        return value;
    }
    inline const std::string& COMMON_SPECIFIC_PATHS()
    {
        static const std::string value = "common_specific_paths";
        return value;
    }
    inline const std::string& COMMON()
    {
        static const std::string value = "common";
        return value;
    }
    inline const std::string& SPECIFIC()
    {
        static const std::string value = "specific";
        return value;
    }
    inline const std::string& TYPE()
    {
        static const std::string value = "type";
        return value;
    }
    inline const std::string& PATH()
    {
        static const std::string value = "path";
        return value;
    }
    inline const std::string& COMBINED()
    {
        static const std::string value = "combined";
        return value;
    }
    inline const std::string& BUILD_SCRIPT_FILE_NAME()
    {
        static const std::string value = "build.cj";
        return value;
    }
    inline const std::string& IMPORT()
    {
        static const std::string value = "import";
        return value;
    }
    inline const std::string& AS()
    {
        static const std::string value = "as";
        return value;
    }
    inline const std::string& WHITE_SPACE()
    {
        static const std::string value = " ";
        return value;
    }
    inline const std::string& COMMA()
    {
        static const std::string value = ":";
        return value;
    }
    inline const std::set<std::string>& BUILTIN_OPERATORS()
    {
        static const std::set<std::string> value = {
            "@", ".", "[]", "()", "++", "--", "?", "!", "-", "**", "*", "/",
            "%", "+", "<<", ">>", "<", "<=", ">", ">=", "is", "as", "==",
            "!=", "&", "^", "|", "..", "..=", "&&", "||", "\?\?",
            "~>", "=", "**=", "*=", "/=", "%/", "+=", "-=", "<<=", ">>=",
            "&=", "^=", "|="
        };
        return value;
    }
    inline const std::string& CANGJIE_FILE_EXTENSION()
    {
        static const std::string value = "cj";
        return value;
    }
    inline const std::string& CANGJIE_MACRO_FILE_EXTENSION()
    {
        static const std::string value = "macrocall";
        return value;
    }
    const unsigned int AD_OFFSET = 2;
    constexpr size_t MAC_THREAD_STACK_SIZE = 1024 * 1024 * 8;
    const unsigned int SORT_TEXT_SIZE = 6;
    const float ROUND_NUM = 0.5;
    inline const std::string& DOUBLE_COLON()
    {
        static const std::string value = "::";
        return value;
    }
} // namespace CONSTANTS

#endif // LSPSERVER_CONSTANTS_H
