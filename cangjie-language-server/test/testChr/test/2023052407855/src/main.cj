// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public open class A22 {
    public open func size1_1(a: Int64, b!: Float16): Unit {}
}

class BA8 <: A22 {
    public override func size1_1(a: Int64, b!: Float16): Unit {}
}

func test39() {
    var a = BA8().size1_1(34, b: 6.7)


}

