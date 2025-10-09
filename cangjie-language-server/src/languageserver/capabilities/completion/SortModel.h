// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_SORTMODEL_H
#define LSPSERVER_SORTMODEL_H

#include <algorithm>
#include <cstdint>
#include <string>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>
#include "CompletionImpl.h"

namespace ark {
class SortModel {
public:
    static constexpr double BASE_SCORE = 1.0;
    static constexpr double LENGTH_WEIGHT = 0.3;
    static constexpr double PREFIX_POSITION_WEIGHT = 0.4;
    static constexpr double CONTINUITY_WEIGHT = 0.3;

    explicit SortModel(double edw = 0.4, double spw = 0.3, double stw = 0.1, double ufw = 0.2)
        : editDistanceWeight(edw), scopePathWeight(spw), symbolTypeWeight(stw), usageFrequencyWeight(ufw)
    {
    }

    void UpdateUsageFrequency(const std::string &item) { usageFrequency[item]++; }

    double CalculateScore(const CodeCompletion &item, const std::string &prefix, uint8_t cursorDepth);

private:
    double editDistanceWeight;
    double scopePathWeight;
    double symbolTypeWeight;
    double usageFrequencyWeight;

    std::unordered_map<std::string, int> usageFrequency;

    static double CalculateMatchScore(std::string_view completion, std::string_view prefix);

    static double CalculateSymbolTypeScore(SortType type);

    static std::pair<double, double> CalculatePositionAndContinuityScores(std::string_view completion,
        std::string_view prefix);
};
} // namespace ark

#endif // LSPSERVER_SORTMODEL_H
