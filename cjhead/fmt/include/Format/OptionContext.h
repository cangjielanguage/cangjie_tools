// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJFMT_OPTIONCONTEXT_H
#define CJFMT_OPTIONCONTEXT_H

#include "Format/ASTToFormatSource.h"

#include <iostream>
#include <string>

namespace Cangjie::Format {
class OptionContext {
public:
    void SetFmtFilePath(const std::string& fmtFilePath);
    void SetFileOutputPath(const std::string& fileOutputPath);
    void SetFmtDirPath(const std::string& fmtDirPath);
    void SetDirOutputPath(const std::string& dirOutputPath);
    void SetConfigFilePath(const std::string& configFilePath);
    void SetCangjieHome(const std::string& cangjieHome);
    void SetConfigOptions(const FormattingOptions& configOptions);

    [[nodiscard]] const std::string& GetFmtFilePath() const;
    [[nodiscard]] const std::string& GetFileOutputPath() const;
    [[nodiscard]] const std::string& GetFmtDirPath() const;
    [[nodiscard]] const std::string& GetDirOutputPath() const;
    [[nodiscard]] const std::string& GetConfigFilePath() const;
    [[nodiscard]] const std::string& GetCangjieHome() const;
    [[nodiscard]] FormattingOptions GetConfigOptions() const noexcept;

    static OptionContext& GetInstance() noexcept
    {
        static OptionContext instance;
        return instance;
    }

private:
    OptionContext() noexcept {};
    ~OptionContext() = default;
    OptionContext(const OptionContext&) = delete;
    OptionContext& operator=(const OptionContext&) = delete;

    std::string m_fmtFilePath;
    std::string m_fileOutputPath;
    std::string m_fmtDirPath;
    std::string m_dirOutputPath;
    std::string m_configFilePath;
    std::string m_cangjieHome;
    FormattingOptions m_configOptions;
};
} // namespace Cangjie::Format
#endif // CJFMT_OPTIONCONTEXT_H
