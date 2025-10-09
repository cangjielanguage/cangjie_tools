// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_COMMON_H
#define LSPSERVER_COMMON_H

#include "URI.h"
#include "cangjie/Basic/Position.h"

namespace ark {
struct URIForFile {
    std::string file = "";
    bool operator==(const URIForFile &rhs) const
    {
        return this->file == rhs.file;
    }
    bool operator!=(const URIForFile &rhs) const
    {
        return !(*this == rhs);
    }
    bool operator<(const URIForFile &rhs) const
    {
        return this->file < rhs.file;
    }
};

struct Range {
    Cangjie::Position start = {0, 0, 0};
    Cangjie::Position end = {0, 0, 0};

    bool operator==(const Range &rhs) const
    {
        return std::tie(this->start, this->end) == std::tie(rhs.start, rhs.end);
    }

    bool operator!=(const Range &rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(const Range &rhs) const
    {
        return std::tie(this->start, this->end) < std::tie(rhs.start, rhs.end);
    }
};

struct Location {
    // The text document's URI.
    URIForFile uri;
    Range range;
    bool operator==(const Location &rhs) const
    {
        return this->uri == rhs.uri && this->range == rhs.range;
    }

    bool operator!=(const Location &rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(const Location &rhs) const
    {
        return std::tie(this->uri, this->range) < std::tie(rhs.uri, rhs.range);
    }
};
};
#endif // LSPSERVER_COMMON_H
