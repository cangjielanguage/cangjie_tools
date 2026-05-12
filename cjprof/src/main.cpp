// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <iostream>
#include <set>
#include "Commands/Command.h"
#include "Commands/Heap.h"
#if defined(__linux__)
#include "Commands/Record.h"
#include "Commands/Report.h"
#endif


static void PrintHelp()
{
#if defined(__linux__)
    std::string help = " Usage: cjprof [--help] COMMAND [ARGS]\n"
        "\n"
        "The supported commands are:\n"
        "  -v        Print version of cjprof\n"
        "  heap      Dump heap into a dump file or analyze the heap dump file\n"
        "  record    Run a command and record its profile data into data file\n"
        "  report    Read profile data file (created by cjprof record) and display the profile\n";
#else
    std::string help = " Usage: cjprof [--help] COMMAND [ARGS]\n"
        "\n"
        "The supported commands are:\n"
        "  -v        Print version of cjprof\n"
        "  heap      Analyze the heap dump file\n";
#endif
    std::cout << help << std::endl;
    return;
}

static void PrintVersion()
{
#ifndef CJPROF_VERSION
    Errorln("Can not obtain cjprof version");
#else
    std::cout << std::string("Cangjie Profile: ") + std::string(CJPROF_VERSION) << std::endl;
#endif
    return;
}

int main(int argc, char **argv)
{
    if ((argc <= 1) || (std::string(argv[1]) == "--help")) {
        PrintHelp();
        return 0;
    }

    if ((std::string(argv[1]) == "-v") || (std::string(argv[1]) == "--version")) {
        PrintVersion();
        return 0;
    }

#if defined(__linux__)
    std::set<Command *> subCmds = { &Heap::GetInstance(), &Record::GetInstance(), &Report::GetInstance() };
#else
    std::set<Command *> subCmds = {&Heap::GetInstance()};
#endif

    for (auto &it : subCmds) {
        if (it->GetName() == argv[1]) {
            return it->Execute(argc - 1, &argv[1]);
        }
    }

    PrintHelp();

    return 0;
}