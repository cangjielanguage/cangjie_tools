// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

@C
public struct FlexibleString {
    public let length: Int32
    public let data: VArray<UInt8, $0>

    public init(length: Int32, data: VArray<UInt8, $0>) {
        this.length = length
        this.data = data
    }
}
