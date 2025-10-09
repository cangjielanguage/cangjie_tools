// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_FILESTORE_H
#define LSPSERVER_FILESTORE_H

#include "Utils.h"

namespace ark {
class FileStore {
public:
    static std::string NormalizePath(const std::string &path);
};
} // namespace ark

#endif // LSPSERVER_FILESTORE_H
