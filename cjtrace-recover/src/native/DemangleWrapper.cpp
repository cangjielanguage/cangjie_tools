// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <string>

#include "CangjieDemangle.h"

std::string Demangle(const std::string &name)
{
    auto demangle = Cangjie::Demangle(name);
    auto fullName = demangle.GetFullName();
    auto pkgName = demangle.GetPkgName();
    auto demangleName = pkgName + (pkgName.size() > 0 ? "." : "") + fullName;
    return demangleName;
}

extern "C" {
#ifdef USE_BOUNDSCHECK_LIBRARY
// we declare here manually to get rid of header dependency
int memcpy_s(char *dest, size_t destMax, const char *src, size_t count);
#endif

const char *Demangle(char *name, size_t len)
{
    std::string mangle;
    mangle.assign(name, name + len);
    auto demangle = Demangle(mangle);
    char *res = static_cast<char*>(malloc(demangle.size() + 1));
    if (res == nullptr) {
        return "";
    }
    memcpy_s(res, demangle.size() + 1, demangle.c_str(), demangle.size());
    res[demangle.size()] = 0;
    return res;
}
}
