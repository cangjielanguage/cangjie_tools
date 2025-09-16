// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

struct Test422{
    let width: int32 = 2
    let length: int32 = 5
    init(a:int32, b:int32) {
        width = a
        length = b
    }

    func area(): int32 {
        return width * length
    }
}

func semanticHighlight_test042():unit {
    var value:Test42 = Test42(1,2)
    var sum = value.area()
    var a = value.length
}
