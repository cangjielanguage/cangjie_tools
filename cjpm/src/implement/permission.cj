// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.fs.*
import stdx.encoding.json.*
import stdx.serialization.serialization.*
import cjpm.config.*

func loadCacheFile(file: String): Option<IncrementalCache> {
    var buf: String = ""
    try {
        buf = String.fromUtf8(File.readFrom(file))
    } catch (e: Exception) {
        return Option<IncrementalCache>.None
    }

    var cache: IncrementalCache = IncrementalCache()
    try {
        cache = IncrementalCache.deserialize(DataModel.fromJson(JsonValue.fromStr(buf)))
    } catch (e: Exception) {
        return Option<IncrementalCache>.None
    }

    return cache
}
