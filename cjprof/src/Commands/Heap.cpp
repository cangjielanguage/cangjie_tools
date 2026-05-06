// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <sys/stat.h>
#include <csignal>
#include <unistd.h>
#include <getopt.h>
#include <fstream>
#include "Analyzer/HeapAnalyzer.h"
#include "Commands/Heap.h"

int Heap::Execute(int argc, char **argv)
{
    if (!ParseArgs(argc, argv)) {
        return ERR;
    }

    if (m_cfg.dump) {
        if (!DumpHeap()) {
            return ERR;
        }

        printf("Heap data is successfully dump to file '%s'.\n", m_cfg.output.c_str());
        return OK;
    }

    auto analyzer = HeapAnalyzer::GetInstance();
    if (!analyzer.SetData(m_cfg.input)) {
        return ERR;
    }

    if (!analyzer.Analyze(m_cfg.verbose)) {
        return ERR;
    }

    if (!m_cfg.showThread && !m_cfg.showReference) {
        analyzer.ShowObject();
    }

    if (m_cfg.showThread) {
        analyzer.ShowThread();
    }

    if (m_cfg.showReference) {
        analyzer.ShowReference(m_cfg.objNameList, m_cfg.maxDepth, m_cfg.incomingRef);
    }

    return OK;
}

static void ParseDepth(const char *arg, int &depth)
{
    char *endPtr = nullptr;
    long pid_long = strtol(arg, &endPtr, 10);
    if (endPtr == arg || *endPtr != '\0' || errno == ERANGE || pid_long <= 0) {
        fprintf(stderr, "error: Invalid pid '%s'.\n", arg);
        return;
    }
    int value = static_cast<int>(pid_long);
    if (value <= 0) {
        printf("warning: Invalid depth value, use the default value (%d).\n", depth);
        return;
    }

    depth = value;
}

bool Heap::ParseArgs(int argc, char **argv)
{
#if defined(__linux__)
    static option longOpts[] = {
        { "depth", required_argument, nullptr, 'D' }, { "dump", required_argument, nullptr, 'd' },
        { "help", no_argument, nullptr, 'h' }, { "incoming-reference", no_argument, nullptr, 1 },
        { "input", required_argument, nullptr, 'i' }, { "output", required_argument, nullptr, 'o' },
        { "show-reference", optional_argument, nullptr, 0 }, { "show-thread", no_argument, nullptr, 't' },
        { "verbose", no_argument, nullptr, 'V' }, { 0, 0, nullptr, 0 }
    };
#else
    static option longOpts[] = {
        {"depth", required_argument, nullptr, 'D'},
        {"help", no_argument, nullptr, 'h'},
        {"incoming-reference", no_argument, nullptr, 1},
        {"input", required_argument, nullptr, 'i'},
        {"show-reference", optional_argument, nullptr, 0},
        {"show-thread", no_argument, nullptr, 't'},
        {"verbose", no_argument, nullptr, 'V'},
        {0, 0, nullptr, 0}};
#endif
    bool interrupted = false;
    while (!interrupted) {
#if defined(__linux__)
        int c = getopt_long(argc, argv, "D:d:hi:o:r::tV", longOpts, nullptr);
#else
        int c = getopt_long(argc, argv, "D:hi:tV", longOpts, nullptr);
#endif
        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                m_cfg.showReference = true;
                m_cfg.objNameList = optarg ?: "";
                break;

            case 1:
                m_cfg.incomingRef = true;
                break;

            case 'D':
                ParseDepth(optarg, m_cfg.maxDepth);
                break;

            case 'd':
            {
                m_cfg.dump = true;
                char *endPtr = nullptr;
                long pid_long = strtol(optarg, &endPtr, 10);
                if (endPtr == optarg || *endPtr != '\0' || errno == ERANGE || pid_long <= 0) {
                    fprintf(stderr, "error: Invalid pid '%s'.\n", optarg);
                    break;
                }
                m_cfg.pid = static_cast<pid_t>(pid_long);
                break;
            }

            case 'i':
                m_cfg.input = optarg;
                break;

            case 'o':
                m_cfg.output = optarg;
                break;

            case 't':
                m_cfg.showThread = true;
                break;

            case 'V':
                m_cfg.verbose = true;
                break;

            default:
                interrupted = true;
                break;
        }
    }

    if (interrupted || (optind < argc)) {
        PrintHelp();
        return false;
    }

    return true;
}

void Heap::PrintHelp()
{
#if defined(__linux__)
    static const char *help =
        " Usage: cjprof heap [<options>]\n"
        "\n"
        "    -D, --depth <depth>   max depth to show for references, default to 10, used only with --show-reference\n"
        "    -d, --dump <pid>      dump heap into a dump file\n"
        "    -h, --help            show this help message\n"
        "    -i, --input <file>    input dump file name, default to 'cjprof.data'\n"
        "    -o, --output <file>   output dump file name, default to 'cjprof.data'\n"
        "        --show-reference[=<objnames>]\n"
        "                          show objects with outgoing references\n"
        "        --incoming-reference\n"
        "                          show incoming references rather than outgoings, used only with --show-reference\n"
        "    -t, --show-thread     show cangjie threads and stacks with objects references\n"
        "    -V, --verbose         print parsing logs\n";
#else
    static const char *help =
        " Usage: cjprof heap [<options>]\n"
        "\n"
        "    -D, --depth <depth>   max depth to show for references, default to 10, used only with --show-reference\n"
        "    -h, --help            show this help message\n"
        "    -i, --input <file>    input dump file name, default to 'cjprof.data'\n"
        "        --show-reference[=<objnames>]\n"
        "                          show objects with outgoing references\n"
        "        --incoming-reference\n"
        "                          show incoming references rather than outgoings, used only with --show-reference\n"
        "    -t, --show-thread     show cangjie threads and stacks with objects references\n"
        "    -V, --verbose         print parsing logs\n";
#endif
    printf("%s\n", help);
}

bool Heap::DumpHeap()
{
#if defined(_WIN32) || defined(__APPLE__)
    return false;
#else
    std::string pidPath = "/proc/" + std::to_string(m_cfg.pid) + "/cwd";
    std::string pidFile = pidPath + m_dump_cfg.hprofFilePath;

    std::string softLinkPath = GetRealpathOfPid();
    if (softLinkPath.empty()) {
        fprintf(stderr, "error: Invalid pid.\n");
        return false;
    }
    std::string softLinkFile = softLinkPath + m_dump_cfg.hprofFilePath;

    struct stat sb = {0};
    if (stat(softLinkFile.c_str(), &sb) == 0) {
        unlink(softLinkFile.c_str());
    }

    if ((m_cfg.pid <= 0) || (kill(m_cfg.pid, m_dump_cfg.signalCode) < 0)) {
        fprintf(stderr, "error: Invalid pid.\n");
        return false;
    }

    printf("Dumping heap ...\n");
    std::string data;
    if (stat(pidPath.c_str(), &sb) == 0) {
        data = pidFile;
    } else if (stat(softLinkPath.c_str(), &sb) == 0) {
        data = softLinkFile;
    } else {
        fprintf(stderr, "error: Invalid path of %s.\n", softLinkPath.c_str());
        return false;
    }
    while (stat(data.c_str(), &sb) != 0) {
        sleep(1);
    }

    if (rename(data.c_str(), m_cfg.output.c_str()) == 0) {
        return true;
    }

    std::ifstream ifs(data);
    std::ofstream ofs(m_cfg.output);
    if (ifs.fail() || ofs.fail()) {
        fprintf(stderr, "error: Cannot create file '%s'.\n", m_cfg.output.c_str());
        return false;
    }

    ifs.seekg(0, ifs.end);
    auto size = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    auto buf = new char[size];
    ifs.read(buf, size);
    ofs.write(buf, size);
    delete [] buf;

    ifs.close();
    ofs.close();

    unlink(data.c_str());

    return true;
#endif
}

std::string Heap::GetRealpathOfPid()
{
#if defined(_WIN32) || defined(__APPLE__)
    return "";
#else
    auto data = "/proc/" + std::to_string(m_cfg.pid) + "/cwd";

    struct stat st = {0};
    if (stat(data.c_str(), &st) == -1) {
        return "";
    }

    auto size = 1024; // define default max path length
    char linkpath[size];
    auto len = readlink(data.c_str(), linkpath, size - 1);
    if (len == -1) {
        return "";
    }
    linkpath[len] = '\0';

    return std::string(linkpath);
#endif
}
