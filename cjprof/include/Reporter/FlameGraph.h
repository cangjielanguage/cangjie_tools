// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef FLAME_GRAPH_H
#define FLAME_GRAPH_H

#include <vector>
#include <tuple>
#include <map>
#include "Utility/Singleton.h"
#include "Reporter.h"

class FlameGraph : public Reporter, public Singleton<FlameGraph> {
    friend Singleton<FlameGraph>;

public:
    void Report() override;

private:
    /* 默认图片宽度 1200 px， 高度 16 px，字体大小 12，字体宽度 0.59 px，最小显示宽度 0.1 px */
    FlameGraph() : m_imageWidth(1200), m_frameHeight(16), m_fontSize(12),
        m_fontWidth(static_cast<float>(0.59)), m_minWidth(static_cast<float>(0.1)) {}
    void GenSortedStacks();
    void MergeStacks();
    std::string ReplaceVariable(const std::string &ori,
        const std::vector<std::pair<std::string, std::string>> &replaceMap);
    std::string GenFrames(float sideMargin, float bottomMargin, int times, float imageHeight);

private:
    using FuncName = std::string;
    using Times = int;
    using StartTime = int;
    using EndTime = int;
    using StackDepth = int;
    using Stack = std::vector<std::string>;

    std::vector<std::pair<Stack, Times>> m_sortedStacks;
    std::vector<std::tuple<FuncName, StackDepth, StartTime, EndTime>> m_mergedStackNodes;
    static const std::string m_svgTemplate;

    float m_imageWidth;
    float m_frameHeight;
    float m_fontSize;
    float m_fontWidth;
    float m_minWidth;
};

#endif // FLAME_GRAPH_H