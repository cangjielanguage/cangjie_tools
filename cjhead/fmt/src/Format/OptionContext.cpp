// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "Format/OptionContext.h"
#include <string>
using namespace Cangjie::Format;

void OptionContext::SetFmtFilePath(const std::string& fmtFilePath)
{
    this->m_fmtFilePath = fmtFilePath;
}

void OptionContext::SetFileOutputPath(const std::string& fileOutputPath)
{
    this->m_fileOutputPath = fileOutputPath;
}

void OptionContext::SetFmtDirPath(const std::string& fmtDirPath)
{
    this->m_fmtDirPath = fmtDirPath;
}

void OptionContext::SetDirOutputPath(const std::string& dirOutputPath)
{
    this->m_dirOutputPath = dirOutputPath;
}

void OptionContext::SetConfigFilePath(const std::string& configFilePath)
{
    this->m_configFilePath = configFilePath;
}

void OptionContext::SetCangjieHome(const std::string& cangjieHome)
{
    this->m_cangjieHome = cangjieHome;
}

void OptionContext::SetConfigOptions(const FormattingOptions& configOptions)
{
    this->m_configOptions = configOptions;
}

const std::string& OptionContext::GetFmtFilePath() const
{
    return m_fmtFilePath;
}

const std::string& OptionContext::GetFileOutputPath() const
{
    return m_fileOutputPath;
}

const std::string& OptionContext::GetFmtDirPath() const
{
    return m_fmtDirPath;
}

const std::string& OptionContext::GetDirOutputPath() const
{
    return m_dirOutputPath;
}

const std::string& OptionContext::GetConfigFilePath() const
{
    return m_configFilePath;
}

const std::string& OptionContext::GetCangjieHome() const
{
    return m_cangjieHome;
}

FormattingOptions OptionContext::GetConfigOptions() const noexcept
{
    return m_configOptions;
}

