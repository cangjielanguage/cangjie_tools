// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef RECORDER_H
#define RECORDER_H

#include <cstdint>
#include <string>
#include <sys/types.h>

class Recorder {
public:
    struct Cfg {
        enum Mode {
            ATTACH,
            LAUNCH,
            INVALID
        };

        Mode mode = INVALID;
        union {
            pid_t pid;
            char **argv;
        };
        /* 默认频率 5000 HZ */
        uint64_t freq = 5000;
        std::string output = "cjprof.data";
    };

public:
    Recorder() : m_cfg() {}
    virtual bool Start() = 0;
    virtual void Stop() = 0;
    void Configure(const Cfg &cfg)
    {
        m_cfg = cfg;
    }
    virtual ~Recorder() = default;

protected:
    Cfg m_cfg;
};

#endif // RECORDER_H