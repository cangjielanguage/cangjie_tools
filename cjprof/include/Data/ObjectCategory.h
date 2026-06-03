// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef OBJECT_CATEGORY_H
#define OBJECT_CATEGORY_H

#include <cstdint>

// Object category - shared by Hprof parser and Analyzer
enum class ObjectCategory : uint8_t {
    INSTANCE_OBJECT = 0,
    OBJECT_ARRAY = 1,
    STRUCT_ARRAY = 2,
    PRIMITIVE_ARRAY = 3,
    PINNED_OBJECT = 4,
    LARGE_OBJECT = 5,
    UNMOVABLE_OBJECT = 6
};

#endif // OBJECT_CATEGORY_H