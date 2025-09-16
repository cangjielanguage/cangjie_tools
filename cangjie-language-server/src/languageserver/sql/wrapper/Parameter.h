// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#pragma once

#include "Bind.h"

#include <stdexcept>
#include <string>

namespace sqldb {

/**
 * Named SQL parameter.
 */
template <typename T>
struct Parameter {
    const char *Name;
    const T &Value;
};

#ifndef DOXYGEN_IGNORE

template <typename T>
struct traits::Bind<Parameter<T>> {
    static int call(sqlite3_stmt *S, int &I, const Parameter<T> &V)
    {
#ifndef NO_EXCEPTIONS
        if ((I = sqlite::bind_parameter_index(S, V.Name)) == 0) {
            throw std::invalid_argument("Invalid named parameter: " + std::string(V.Name));
        }
#else
        I = sqlite::bind_parameter_index(S, V.Name);
#endif
        return traits::bind(S, I, V.Value);
    }
};

#endif // DOXYGEN_IGNORE

} // namespace sqldb