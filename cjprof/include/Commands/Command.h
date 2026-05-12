// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <sys/types.h>

class Command {
public:
    enum {
        ERR = -1,
        OK = 0
    };

public:
    explicit Command(const std::string &name) : m_name(name) {}

    const std::string &GetName() const
    {
        return m_name;
    }

    virtual int Execute(int argc, char **argv) = 0;
    virtual void PrintHelp() = 0;
    virtual ~Command() = default;

private:
    std::string m_name;
};

#endif // COMMAND_H