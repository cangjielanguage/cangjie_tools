// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_LRUCACHE_H
#define LSPSERVER_LRUCACHE_H

#include "../../LSPCompilerInstance.h"
#ifdef __linux__
#include <malloc.h>
#endif
#if __APPLE__
#include <malloc/malloc.h>
#endif

namespace ark {
using namespace Cangjie;
using CacheListIter = std::list<std::pair<std::string, std::unique_ptr<LSPCompilerInstance>>>::iterator;

class LRUCache {
public:
    explicit LRUCache(size_t capacity) : size(capacity) {};

    ~LRUCache() = default;

    bool HasCache(const std::string &key)
    {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return lruHashMap.find(key) != lruHashMap.end();
    }

    std::vector<std::string> GetMpKey()
    {
        std::shared_lock<std::shared_mutex> lock(mtx);
        std::vector<std::string> mapKey = {};
        for (const auto &item : lruHashMap) {
            mapKey.push_back(item.first);
        }
        return mapKey;
    }

    void EraseCache(const std::string &key)
    {
        std::unique_lock<std::shared_mutex> lock(mtx);
        if (lruHashMap.find(key) == lruHashMap.end()) { return; }
        lruHashMap[key]->second.reset(nullptr);
        (void) lruList.erase(lruHashMap[key]);
        (void) lruHashMap.erase(key);
#ifdef __linux__
        (void) malloc_trim(0);
#elif __APPLE__
        (void) malloc_zone_pressure_relief(malloc_default_zone(), 0);
#endif
    }

    std::unique_ptr<Cangjie::LSPCompilerInstance> &Get(const std::string &key)
    {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (lruHashMap.find(key) == lruHashMap.end()) { return nullLSPCompilerInstance; }
        auto ret = lruHashMap.find(key);
        auto it = ret->second;
        lruList.splice(lruList.begin(), lruList, it);
        return it->second;
    }

    std::string Set(const std::string &key, std::unique_ptr<Cangjie::LSPCompilerInstance> &value)
    {
        std::unique_lock<std::shared_mutex> lock(mtx);
        // update cache
        std::string deleteKey;
        auto ret = lruHashMap.find(key);
        if (ret != lruHashMap.end()) {
            // Because deleting the compiler instance is time-consuming
            // we release the pointer here and delete it asynchronously later to avoid the time cost.
            auto ci = ret->second->second.release();
            ret->second->second = std::move(value);
            lruList.splice(lruList.begin(), lruList, ret->second);
            std::thread deleteCI([ci]() {
                delete ci;
            });
            deleteCI.detach();
#ifdef __linux__
            (void) malloc_trim(0);
#elif __APPLE__
            (void) malloc_zone_pressure_relief(malloc_default_zone(), 0);
#endif
            return deleteKey;
        }
        // add cache
        if (lruHashMap.size() >= size) {
            deleteKey = lruList.back().first;
            if (lruHashMap.find(deleteKey) != lruHashMap.end()) {
                // Because deleting the compiler instance is time-consuming
                // we release the pointer here and delete it asynchronously later to avoid the time cost.
                auto ci = lruHashMap[deleteKey]->second.release();
                (void) lruList.erase(lruHashMap[deleteKey]);
                (void) lruHashMap.erase(deleteKey);
                std::thread deleteCI([ci]() {
                    delete ci;
                });
                deleteCI.detach();
            }
        }
#ifdef __linux__
        (void) malloc_trim(0);
#elif __APPLE__
        (void) malloc_zone_pressure_relief(malloc_default_zone(), 0);
#endif
        (void) lruList.emplace_front(key, std::move(value));
        lruHashMap[key] = lruList.begin();
        return deleteKey;
    }

    void SetForFullCompiler(const std::string &key, std::unique_ptr<Cangjie::LSPCompilerInstance> &value)
    {
        std::unique_lock<std::shared_mutex> lock(mtx);
        if (lruHashMap.size() <= size) {
            (void) lruList.emplace_front(key, std::move(value));
            lruHashMap[key] = lruList.begin();
        } else {
            value.reset(nullptr);
#ifdef __linux__
            (void) malloc_trim(0);
#elif __APPLE__
            (void) malloc_zone_pressure_relief(malloc_default_zone(), 0);
#endif
        }
    }
    std::unique_ptr<Cangjie::LSPCompilerInstance> nullLSPCompilerInstance = nullptr;

private:
    size_t size;
    std::unordered_map<std::string, CacheListIter> lruHashMap{};
    std::list<std::pair<std::string, std::unique_ptr<Cangjie::LSPCompilerInstance>>> lruList{};
    std::shared_mutex mtx;
};
} // namespace ark

#endif // LSPSERVER_LRUCACHE_H
