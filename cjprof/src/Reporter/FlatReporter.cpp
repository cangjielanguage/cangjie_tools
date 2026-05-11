// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <map>
#include <algorithm>
#include <tuple>
#include "Reporter/FlatReporter.h"

void FlatReporter::Report()
{
    size_t total = 0;
    std::map<std::string, std::tuple<size_t, size_t>> funcs;
    for (auto sample : m_samplesData) {
        total += sample.num;
        for (auto func : sample.stackTrace) {
            if (funcs.find(func) == funcs.end()) {
                funcs[func] = std::make_tuple<size_t, size_t>(0, 0);
            }

            std::get<0>(funcs[func]) += sample.num;
        }

        auto self = sample.stackTrace.front();
        std::get<1>(funcs[self]) += sample.num;
        if (sample.stackTrace.size() == 1) {
            std::get<0>(funcs[self]) -= sample.num;
        }
    }

    std::vector<std::pair<std::string, std::tuple<size_t, size_t>>> vec(funcs.begin(), funcs.end());
    auto comp = [](const std::pair<std::string, std::tuple<size_t, size_t>> &a,
        const std::pair<std::string, std::tuple<size_t, size_t>> &b) {
        if (std::get<0>(a.second) == std::get<0>(b.second)) {
            return std::get<1>(a.second) > std::get<1>(b.second);
        }
        return std::get<0>(a.second) > std::get<0>(b.second);
    };
    std::sort(vec.begin(), vec.end(), comp);

    printf("Children        Self    Symbol\n");
    printf("========    ========    ========\n");

    for (auto func : vec) {
        printf(" %6.2f%%     %6.2f%%    %s\n",
            /* Printf the percentage of children and self as format of 100.00%. */
            static_cast<double>(std::get<0>(func.second)) * 100.00 / static_cast<double>(total),
            static_cast<double>(std::get<1>(func.second)) * 100.00 / static_cast<double>(total), func.first.c_str());
    }
}