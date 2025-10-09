// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "SortModel.h"
#include <cstdint>
#include "CompletionImpl.h"

namespace ark {
constexpr double LOW_SCORE_LIMIT = 0.3;
constexpr double KEYWORD_WEIGHT = 1.0;
constexpr double NORMAL_SYM_WEIGHT = 0.8;
constexpr double AUTO_IMPORT_SYM_WEIGHT = 0.6;

double SortModel::CalculateScore(const CodeCompletion &item, const std::string &prefix, uint8_t cursorDepth)
{
    double matchScore = CalculateMatchScore(item.name, prefix);
    double scopePathScore = 1.0 / (1.0 + std::abs(cursorDepth - item.itemDepth));
    double symbolTypeScore = CalculateSymbolTypeScore(item.sortType);
    int frequency = usageFrequency[item.label];
    double usageFrequencyScore = frequency > 0 ? 1.0 - 1.0 / (1.0 + frequency) : 0;
    double score = editDistanceWeight * matchScore + scopePathWeight * scopePathScore +
                   symbolTypeWeight * symbolTypeScore + usageFrequencyWeight * usageFrequencyScore;
    return score;
}


double SortModel::CalculateMatchScore(std::string_view completion, std::string_view prefix)
{
    if (prefix.empty()) {
        return BASE_SCORE;
    }
    auto [positionScore, continuityScore] = CalculatePositionAndContinuityScores(completion, prefix);
    double lengthScore = static_cast<double>(prefix.length()) / completion.length();
    double combinedScore = LENGTH_WEIGHT * lengthScore +
                           PREFIX_POSITION_WEIGHT * positionScore +
                           CONTINUITY_WEIGHT * continuityScore;
    return combinedScore;
}

std::pair<double, double> SortModel::CalculatePositionAndContinuityScores(
    std::string_view completion, std::string_view prefix)
{
    size_t maxContinuousMatch = 0;
    size_t currentContinuousMatch = 0;
    size_t totalMatch = 0;
    double averagePosition = 0;

    size_t prefixIndex = 0;
    for (size_t i = 0; i < completion.length() && prefixIndex < prefix.length(); ++i) {
        if (completion[i] == prefix[prefixIndex]) {
            ++currentContinuousMatch;
            maxContinuousMatch = std::max(maxContinuousMatch, currentContinuousMatch);
            averagePosition += i;
            ++totalMatch;
            ++prefixIndex;
        } else {
            currentContinuousMatch = 0;
        }
    }

    double positionScore = 1.0;
    double continuityScore = 0.0;

    if (totalMatch > 0) {
        averagePosition /= totalMatch;
        positionScore = 1.0 - (averagePosition / completion.length());
        continuityScore = static_cast<double>(maxContinuousMatch) / prefix.length();
    }

    return {positionScore, continuityScore};
}

double SortModel::CalculateSymbolTypeScore(SortType type)
{
    switch (type) {
        case SortType::KEYWORD:
            return KEYWORD_WEIGHT;
        case SortType::NORMAL_SYM:
            return NORMAL_SYM_WEIGHT;
        case SortType::AUTO_IMPORT_SYM:
            return AUTO_IMPORT_SYM_WEIGHT;
    }

    return LOW_SCORE_LIMIT;
}
} // namespace ark