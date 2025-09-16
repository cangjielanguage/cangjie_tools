// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_CJO_MANAGER_H
#define LSPSERVER_CJO_MANAGER_H

#include <cstdint>
#include <iostream>
#include <set>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "DependencyGraph.h"
#include "logger/Logger.h"

namespace ark {
enum class DataStatus: uint8_t {
    FRESH,
    // Indicates that the package status may be stale, whether compilation is required is determined at runtime
    WEAKSTALE,
    STALE,
};

using SerializedT = std::vector<uint8_t>;

struct CjoData {
    std::optional<SerializedT> data;
    DataStatus status;
};

class CjoManager {
public:
    // copy data
    void SetData(const std::string &fullPkgName, CjoData data)
    {
        std::unique_lock lock(mutex);
        auto ret = cjoMap.insert_or_assign(fullPkgName, data);
        if (ret.second) {
            Trace::Log("Insert cjo cache of package ", fullPkgName);
        } else {
            Trace::Log("Update cjo cache of package ", fullPkgName);
        }
    }

    // return a shared_ptr
    std::shared_ptr<SerializedT> GetData(const std::string &fullPkgName)
    {
        std::shared_lock lock(mutex);
        auto it = cjoMap.find(fullPkgName);
        if (it != cjoMap.end()) {
            if (it->second.data.has_value()) {
                return std::make_shared<SerializedT>(it->second.data.value());
            }
        }
        return nullptr;
    }

    /**
     * @brief Update the cache status of the package. Usually, after the astdata of this package is exported,
     * the cjo cache of the downstream package is updated to Stale.
     *
     * @param packages  Packages that change state
     * @param status  the state needs to be changed
     */
    void UpdateStatus(const std::unordered_set<std::string> &packages, DataStatus status)
    {
        std::unique_lock lock(mutex);
        for (auto &package : packages) {
            if (cjoMap.find(package) != cjoMap.end()) {
                if (cjoMap[package].status == DataStatus::STALE && status == DataStatus::WEAKSTALE) {
                    Trace::Log("package status is stale, cann't change to weak stale", package);
                    continue;
                }
                cjoMap[package].status = status;
                if (status == DataStatus::STALE) {
                    Trace::Log(package + "'s cjo cache is stale");
                } else if (status == DataStatus::WEAKSTALE) {
                    Trace::Log(package + "'s cjo cache is weak stale");
                } else {
                    Trace::Log(package + "'s cjo cache is fresh");
                }
            }
        }
    }

    /**
     * @brief Check whether the cjo cache corresponding to the package is fresh.
     *
     * @param packages The set of packages to be checked is usually
     * the upstream packages of the packages to be compiled.
     * @return std::unordered_set<std::string> The cjo cache is a collection of
     * stale packages that need to be recompiled.
     */
    std::unordered_set<std::string> CheckStatus(const std::unordered_set<std::string> &packages)
    {
        std::shared_lock lock(mutex);
        std::unordered_set<std::string> result;
        for (auto &package : packages) {
            if (cjoMap.find(package) != cjoMap.end() && cjoMap[package].status != DataStatus::FRESH) {
                    result.emplace(package);
            }
        }
        return result;
    }

    DataStatus GetStatus(const std::string& package)
    {
        std::shared_lock lock(mutex);
        if (cjoMap.find(package) != cjoMap.end()) {
            return cjoMap[package].status;
        }
        return DataStatus::STALE;
    }

    bool CheckChanged(const std::string& fullPkgName, const SerializedT& data)
    {
        std::shared_lock lock(mutex);
        if (cjoMap.find(fullPkgName) == cjoMap.end()) {
            return true;
        }
        auto oldData = GetData(fullPkgName);
        return !oldData || data != *oldData;
    }

    void UpdateDownstreamPackages(const std::string& package, const std::unique_ptr<DependencyGraph>& graph)
    {
        auto downPackages = graph->FindAllDependents(package);
        auto directDownPackages = graph->FindMayDependents(package);
        UpdateStatus(directDownPackages, DataStatus::STALE);
        UpdateStatus(downPackages, DataStatus::WEAKSTALE);
    }

private:
    mutable std::shared_mutex mutex;
    std::unordered_map<std::string, CjoData> cjoMap;
};
} // namespace ark
#endif // LSPSERVER_CJO_MANAGER_H