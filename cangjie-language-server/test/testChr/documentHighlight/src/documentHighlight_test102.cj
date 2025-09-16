// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class Point1 {
    var x: int32 = 0
    var y: int32 = 0
    init (a: int32, b: int32) {
        x = a
        y = b
    }
    operator func -(operand: Point1): Point1 { Point1(-operand.x, -operand.y) }
    operator func ~(operand: Point1): Point1 { Point1(-operand.x, -operand.y) }
    operator func **(left: Point1, right: Point1): Point1  = Point1(left.x + right.x, left.y + right.y)
}

func documentHighlight_test102() :unit{
    var p1 = Point1(8, 24)
    var p2 = Point1(10,32)
    let p3 = -p1
    let p4 = ~p1
    let p5 = p1 ** p2
}
