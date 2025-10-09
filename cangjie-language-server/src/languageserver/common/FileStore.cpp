// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include "FileStore.h"

#ifndef __linux__
#include <string>
#endif

#ifdef _WIN32
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#elif defined __linux__

#include <dirent.h>

#endif
namespace ark {
#ifdef _WIN32
void LowAbsFileName(char absPathBuff[], size_t len)
{
    if (len == 0) { return; }
    if (FileUtil::GetFileExtension(std::string(absPathBuff)) != "cj") { return; }
    for (int i = static_cast<int>(len) - 1; i >= 0; i--) {
        if (absPathBuff[i] == '/' || absPathBuff[i] == '\\') { break; }
        if (isupper(absPathBuff[i])) {
            absPathBuff[i] = tolower(absPathBuff[i]);
        }
    }
}
#endif

std::string FileStore::NormalizePath(const std::string &path)
{
    if (path.length() >= PATH_MAX) {
        return path;
    }
    char absPathBuff[PATH_MAX] = {0};
#ifdef _WIN32
    _fullpath(absPathBuff, path.c_str(), PATH_MAX);
    // we use lower drive letter because cangjie generate fileHash by filePath
    absPathBuff[0] = tolower(absPathBuff[0]);
    LowAbsFileName(absPathBuff, std::string(absPathBuff).length());
#else
    if (realpath(path.c_str(), absPathBuff) == nullptr) {
        return path;
    }
#endif
    std::string str = std::string(absPathBuff);
#ifdef __linux__
    for (auto &s: str) {
        if (s == '\\') {
            s = '/';
        }
    }
#endif
    return str;
}
} // namespace ark
