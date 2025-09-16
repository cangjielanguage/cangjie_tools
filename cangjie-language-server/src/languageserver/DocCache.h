// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_DOCCACHE_H
#define LSPSERVER_DOCCACHE_H

#include <string>
#include <map>
#include <mutex>
#include <cstdint>
#include "../json-rpc/Protocol.h"
namespace ark {
class DocCache {
public:
    struct Doc {
        std::string contents = "";
        std::int64_t version = -1;
        bool needReParser = false;
        bool isInitCompiled = false;
    };

    std::int64_t AddDoc(const std::string &file, int64_t version, std::string contents);

    void AddDocWhenInitCompile(const std::string &file);

    void RemoveDoc(const std::string &file);

    Doc GetDoc(const std::string &file);

    bool UpdateDoc(const std::string &file, std::int64_t version, bool needReParser,
                   const std::vector<TextDocumentContentChangeEvent> &contentChanges);

    void UpdateDocNeedReparse(const std::string &file, int64_t version, bool needReParser);

private:
    mutable std::mutex mutex;
    std::map<std::string, Doc> Docs;
};
} // namespace ark

#endif // LSPSERVER_DOCCACHE_H
