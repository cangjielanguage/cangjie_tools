// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package document_highlight
class AA {
    
}

class BB {
    
}

class Test<T> {
    let start: Int64
    let len: Int64
    public init(size: Int64, repeat!: T) {
        this.start = 0
        this.len = size
    }
    public init(size: Int64, initElement: (Int64) -> T) {
        this.start = 0
        this.len = size
    }
    func name(x1: Int64, y1: Float64) {
        var a: Int32 = 10
        var b: Int32 = 20
        return a + b
    }
    func name(x1: Bool, y1: Int64) {}
    func name(a: AA, b: BB) {}
    func name(x1: Bool, y1: Int32, z1!: Float64) {}
    func name(x1: Int32) {}
}

main(): Int64 {
    var x1 = Test<Int64>(
        8,
        {
            i => i + 1
        }
    )
    var x4 = x1.name(1, 6.2)
    var x5 = x1.name(true, 8)
    var x6 = x1.name(true, 8, z1: 6.6)
    var a = AA()
    var b = BB()
    var x7 = x1.name(a, b)
    var x8 = x1.name(1)
    return 0
}
