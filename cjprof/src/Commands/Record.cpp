// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <climits>
#include <getopt.h>
#include <fstream>
#include "Symbol/Elf.h"
#include "Commands/Record.h"
#include "Recorder/LinuxRecorder.h"

int Record::Execute(int argc, char **argv)
{
    Recorder::Cfg cfg;
    if (!ParseArgs(argc, argv, cfg)) {
        return ERR;
    }

    Recorder *recorder = &LinuxRecorder::GetInstance();

    recorder->Configure(cfg);
    if (!recorder->Start()) {
        return ERR;
    }

    recorder->Stop();

    return OK;
}

void Record::PrintHelp()
{
    static const char *help =
        " Usage: cjprof record [<options>] [<command>]\n"
        "    or: cjprof record [<options>] -- <command> [<options>]\n"
        "\n"
        "    -f, --freq <freq>     profile at this frequency\n"
        "    -h, --help            show this help message\n"
        "    -o, --output <file>   output file name, default to 'cjprof.data'\n"
        "    -p, --pid <pid>       record events on existing process id\n";
    printf("%s\n", help);
}

static void ParseFreq(const std::string &arg, uint64_t &freq)
{
    uint64_t maxFreq = ULONG_MAX;
    std::ifstream ifs("/proc/sys/kernel/perf_event_max_sample_rate");
    if (!ifs.fail()) {
        std::string line;
        std::getline(ifs, line);
        maxFreq = std::stoul(line);

        ifs.close();
    }

    if (arg == "max") {
        freq = maxFreq;
        printf("info: Using a maximum frequency rate of %lu Hz.\n", freq);

        return;
    }
    
    char *endPtr = nullptr;
    long value = strtol(arg.c_str(), &endPtr, 10);
    if (endPtr == arg.c_str() || *endPtr != '\0' || errno == ERANGE || value <= 0) {
        value = static_cast<long>(freq);
        printf("warning: Invalid frequency value, use the default value (%ld Hz).\n", value);
        return;
    }

    if (value > static_cast<long>(maxFreq)) {
        freq = maxFreq;
        printf("warning: Maximum frequency rate (%lu Hz) exceeded, throttling from %ld Hz to %lu Hz.\n",
            maxFreq, value, maxFreq);
        return;
    }

    freq = static_cast<uint64_t>(value);
}

bool Record::ParseArgs(int argc, char **argv, Recorder::Cfg &cfg)
{
    static option longOpts[] = {
        { "freq", required_argument, nullptr, 'f' },
        { "help", no_argument, nullptr, 'h' },
        { "output", required_argument, nullptr, 'o' },
        { "pid", required_argument, nullptr, 'p' },
        { 0, 0, nullptr, 0 }
    };

    bool interrupted = false;
    while (!interrupted) {
        int c = getopt_long(argc, argv, "f:ho:p:", longOpts, nullptr);
        if (c == -1)
            break;

        switch (c) {
            case 'f':
                ParseFreq(optarg, cfg.freq);
                break;

            case 'o':
                cfg.output = optarg;
                break;

            case 'p':
            {
                cfg.mode = Recorder::Cfg::ATTACH;
                char *endPtr = nullptr;
                long pid_long = strtol(optarg, &endPtr, 10);
                if (endPtr == optarg || *endPtr != '\0' || errno == ERANGE || pid_long <= 0) {
                    fprintf(stderr, "error: Invalid pid '%s'.\n", optarg);
                    break;
                }
                cfg.pid = static_cast<pid_t>(pid_long);
                break;
            }

            default:
                interrupted = true;
                break;
        }
    }

    if (!interrupted && (optind < argc)) {
        cfg.mode = Recorder::Cfg::LAUNCH;
        cfg.argv = &argv[optind];
    }

    if (interrupted || (cfg.mode == Recorder::Cfg::INVALID)) {
        PrintHelp();
        return false;
    }

    return true;
}