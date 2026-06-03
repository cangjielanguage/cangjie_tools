// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef REPORTER_H
#define REPORTER_H

#include <cstdint>
#include <string>
#include <vector>

class Reporter {
public:
    struct Cfg {
        enum Type {
            FLAT,
            FLAME_GRAPH,
            INVALID
        };

        Cfg() : type(FLAT), input("cjprof.data"), output("FlameGraph.svg") {}
        Type type;
        std::string input;
        std::string output;
    };

    struct StackTraceSample {
        std::vector<std::string> stackTrace;
        uint32_t num;
    };

    Reporter() : m_cfg() {}
    virtual void Report() = 0;

    void Configure(const Cfg &cfg)
    {
        m_cfg = cfg;
    }

    bool SetSampleData(const std::vector<StackTraceSample> &data)
    {
        m_samplesData = data;
        return true;
    }

    virtual ~Reporter() = default;

protected:
    Cfg m_cfg;
    std::vector<StackTraceSample> m_samplesData;
};

#endif // REPORTER_H