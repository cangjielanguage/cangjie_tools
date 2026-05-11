// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef RECORD_H
#define RECORD_H

#include "Commands/Command.h"
#include "Utility/Singleton.h"
#include "Recorder/Recorder.h"

class Record : public Command, public Singleton<Record> {
    friend Singleton<Record>;

public:
    int Execute(int argc, char **argv) override;
    void PrintHelp() override;

private:
    Record() : Command("record") {}
    bool ParseArgs(int argc, char **argv, Recorder::Cfg &cfg);
};

#endif // RECORD_H