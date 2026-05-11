// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef HEAP_H
#define HEAP_H

#include "Commands/Command.h"
#include "Utility/Singleton.h"

class Heap : public Command, public Singleton<Heap> {
    friend Singleton<Heap>;

public:
    int Execute(int argc, char **argv) override;
    void PrintHelp() override;

private:
    Heap();
    bool ParseArgs(int argc, char **argv);
    bool DumpHeap();
    std::string GetRealpathOfPid();

    struct {
        pid_t pid = -1;
        int maxDepth = 10;
        bool dump = false;
        bool verbose = false;
        bool showThread = false;
        bool showReference = false;
        bool incomingRef = false;
        bool dumpReport = false;
        int reportPort = 8080;
        std::string input = "cjprof.data";
        std::string output = "cjprof.data";
        std::string objNameList;
    } m_cfg;

    struct {
        std::string hprofFilePath;
        unsigned int signalCode;
        bool convertFromHprof;
    } m_dump_cfg;
};

#endif // HEAP_H