// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <getopt.h>
#include <fstream>
#include "Data/Hprof.h"
#include "Reporter/FlameGraph.h"
#include "Reporter/FlatReporter.h"
#include "Commands/Report.h"

int Report::Execute(int argc, char **argv)
{
    Reporter::Cfg cfg;
    if (!ParseArgs(argc, argv, cfg)) {
        return ERR;
    }
    std::ifstream ifs(cfg.input);
    char *canonicalPath = realpath(cfg.input.c_str(), nullptr);
    if (canonicalPath == nullptr) {
        return ERR;
    }
    free(canonicalPath);
    if (ifs.fail()) {
        fprintf(stderr, "error: Open file '%s' failed.\n", cfg.input.c_str());
        return ERR;
    }

    ifs.seekg(0, ifs.end);
    auto size = ifs.tellg();
    if (size < 0) {
        fprintf(stderr, "error: Cannot determine '%s' file size.\n", cfg.input.c_str());
        return false;
    }
    ifs.seekg(0, ifs.beg);

    std::vector<char> buf(size);
    ifs.read(buf.data(), size);
    auto data = std::string(buf.data(), size);

    Hprof hprof;
    if (!hprof.Parse(data)) {
        return false;
    }

    auto strings = hprof.GetStrings();
    auto frames = hprof.GetFrames();
    auto stackTraces = hprof.GetStackTraces();
    auto cpuSamples = hprof.GetCpuSamples();
    std::vector<Reporter::StackTraceSample> stackTraceSamples;
    for (auto samples : cpuSamples) {
        for (auto stackTrace : samples.second.stackTraces) {
            stackTraceSamples.push_back({ std::vector<std::string>(), stackTrace.second });
            auto stackTraceIdx = stackTrace.first;
            for (auto frameId : stackTraces[stackTraceIdx].frames) {
                stackTraceSamples.back().stackTrace.push_back(strings[frames[frameId].name]);
            }
        }
    }

    if (stackTraceSamples.empty()) {
        fprintf(stderr, "error: No sampling data is available.\n");
        return ERR;
    }

    Reporter *reporter;
    if (cfg.type == Reporter::Cfg::FLAME_GRAPH) {
        reporter = &FlameGraph::GetInstance();
    } else {
        reporter = &FlatReporter::GetInstance();
    }
    reporter->Configure(cfg);
    reporter->SetSampleData(stackTraceSamples);
    reporter->Report();

    return OK;
}

bool Report::ParseArgs(int argc, char **argv, Reporter::Cfg &cfg)
{
    static option longOpts[] = {
        { "flame-graph", no_argument, nullptr, 'F' },
        { "help", no_argument, nullptr, 'h' },
        { "input", required_argument, nullptr, 'i' },
        { "output", required_argument, nullptr, 'o' },
        { 0, 0, nullptr, 0 }
    };

    bool interrupted = false;
    while (!interrupted) {
        int c = getopt_long(argc, argv, "Fhi:o:", longOpts, nullptr);
        if (c == -1)
            break;

        switch (c) {
            case 'F':
                cfg.type = Reporter::Cfg::FLAME_GRAPH;
                break;

            case 'i':
                cfg.input = optarg;
                break;

            case 'o':
                cfg.output = optarg;
                break;

            default:
                interrupted = true;
                break;
        }
    }

    if ((optind < argc) || interrupted) {
        PrintHelp();
        return false;
    }

    return true;
}

void Report::PrintHelp()
{
    static const char *help =
        " Usage: cjprof report [<options>]\n"
        "\n"
        "    -F, --flame-graph     generate flame graph\n"
        "    -h, --help            show this help message\n"
        "    -i, --input <file>    input file name, default to 'cjprof.data'\n"
        "    -o, --output <file>   output file name, only used for flame graph, default to 'FlameGraph.svg'\n";
    printf("%s\n", help);
}