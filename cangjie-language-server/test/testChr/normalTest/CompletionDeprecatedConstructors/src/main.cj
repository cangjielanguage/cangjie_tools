// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package completion_deprecated

public class StableDeprecatedCtor {
    @Deprecated
    public init() {}
    @Deprecated
    public init(value: Int64) {}
}

main(): Int64 {
    StableDeprecatedCtor
    0
}
