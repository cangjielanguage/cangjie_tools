// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// 操作符重载

class Point1{
    var x: Int32 = 0
    var y: Int32 = 0
    init (a: Int32, b: Int32) {
        x = a
        y = b
    }

    // unary
    operator func -(): Point1 { Point1(-this.x, -this.y) }
    operator func !(): Point1 { Point1(-this.x, -this.y) }

    //binary
    operator func +(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func -(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func *(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func /(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func %(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func **(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func <<(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func >>(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func <(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func <=(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func >(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func >=(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func ==(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func !=(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func &(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func ^(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }
    operator func |(right: Point1): Point1  { Point1(this.x + right.x, this.y + right.y) }

}
func main(): Int64 {
    var p1 = Point1(8, 24)
    var p2 = Point1(10,32)

    var p3 = -p1
    var p4 = !p1

    var p5 = p1 + p2
    var p6 = p1 - p2
    var p7 = p1 * p2
    var p8 = p1 / p2
    var p9 = p1 % p2
    var p10 = p1 ** p2
    var p11 = p1 << p2
    var p12 = p1 >> p2
    var p13 = p1 < p2
    var p14 = p1 <= p2
    var p15 = p1 > p2
    var p16 = p1 >= p2
    var p17 = p1 == p2
    var p18 = p1 != p2
    var p19 = p1 & p2
    var p20 = p1 ^ p2
    var p21 = p1 | p2

    return 0
}

class A {
    var num: Int64 = 0

    operator func [](index: Int64): Int64 {
        return this.num + index
    }

    operator func [](index: Int64, value: Int64): Unit {
        this.num = index + value
    }
}

func main1(): Int64 {
    var a: A = A()
    print("${a[2]}\n") // invoke operator func [](x: A, index: Int64): Int64
    a[2, 5]         // invoke operator func [](x: A, index: Int64, value: Int64): Unit
    print("${a[1]}")
    return 0
}

