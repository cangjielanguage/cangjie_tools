// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LINUX_RECORDER_H
#define LINUX_RECORDER_H

#include <map>
#include <thread>
#include <mutex>
#include <queue>
#include <linux/perf_event.h>
#include "Data/Hprof.h"
#include "Symbol/Symbol.h"
#include "Utility/Singleton.h"
#include "Recorder.h"

class LinuxRecorder : public Recorder, public Singleton<LinuxRecorder> {
    friend Singleton<LinuxRecorder>;
public:
    bool Start() override;
    void Stop() override;

private:
    struct PerfRecordMmap {
        perf_event_header header;
        uint32_t pid;
        uint32_t tid;
        uint64_t addr;
        uint64_t len;
        uint64_t pgoff;
        char filename[];
    };

    struct PerfRecordSample {
        perf_event_header header;
        uint64_t nr;
        uint64_t ips[];
    };

    struct Mmap {
        uint64_t addr;
        uint64_t len;
        std::string file;
        bool dynamic;

        bool operator<(const Mmap &rhs) const
        {
            return (addr + len) <= rhs.addr;
        };
    };

    LinuxRecorder() = default;
    void SampleThreadEntry(pid_t pid);
    void MergeSampleData();
    void AnalyzeSampleData(Hprof &hprof, std::unique_ptr<char[]> data, size_t size);
    void Start(pid_t pid);
    void Stop(pid_t pid);
    void HandleProcessMaps(pid_t pid);
    std::vector<pid_t> TraceProcess();
    bool createDirIfNotExists(const std::string& dirPath);
    void deleteDirIfExists(const std::string& dirPath);

    bool m_stop;
    std::vector<pid_t> m_pids;
    std::map<pid_t, std::thread> m_sampleThreads;
    std::map<pid_t, bool> m_stopSampleThread;
    std::vector<Mmap> m_mmaps;
    std::vector<Symbol> m_symbols;
    std::map<std::string, uint64_t> m_dylibBases;
};

#endif // LINUX_RECORDER_H