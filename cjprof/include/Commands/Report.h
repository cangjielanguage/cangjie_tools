// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef REPORT_H
#define REPORT_H

#include "Commands/Command.h"
#include "Utility/Singleton.h"
#include "Reporter/Reporter.h"

class Report : public Command, public Singleton<Report> {
    friend Singleton<Report>;

public:
    int Execute(int argc, char **argv) override;
    void PrintHelp() override;

private:
    Report() : Command("report") {}
    bool ParseArgs(int argc, char **argv, Reporter::Cfg &cfg);
};

#endif // REPORT_H