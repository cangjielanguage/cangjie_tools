// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testOperatorOverride

internal open class Point {
    var x: int32 = 0
    var y: int32 = 0
    init (a: int32, b: int32) {
        x = a
        y = b
    }

operator func --(left: Point): Point  = Point(-left.x, -left.y )
operator func ++(left: Point): Point  = Point(-left.x, -left.y )
operator func ~(left: Point): Point  = Point(-left.x, -left.y )
    operator func -(left: Point): Point  = Point(-left.x, -left.y )
    operator func !(left: Point): Point  = Point(-left.x, -left.y )
    /**
    * this func is operator override for +
    */
    operator func +(left: Point, right: Point): Point  = Point(left.x + right.x, left.y + right.y)

    /**
    * this func is operator override for -
    */
    operator func -(left: Point, right: Point): Point  = Point(left.x - right.x, left.y - right.y)

    operator func *(left: Point, right: Point): Point  = Point(left.x * right.x, left.y * right.y)
    operator func **(left: Point, right: Point): Point  = Point(left.x * right.x, left.y * right.y)
    operator func /(left: Point, right: Point): Point  = Point(left.x / right.x, left.y / right.y)
    operator func %(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)

    operator func [](left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func %(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func >>(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func <<(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func <(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func <=(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func >(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func >=(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func !=(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func ==(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func &(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func ^(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
    operator func |(left: Point, right: Point): Point  = Point(left.x % right.x, left.y % right.y)
}

/**
* this is a testFunc function
*/
func testFunc1() {
    var p1: Point = Point(1,10)
    var p2: Point = Point(2,11)
    var p3: Point = Point(3, 12)
    p1++
    p1++
    p1--
    !p1
    !p1
    ~p1
    ~p1
    -p1

    p3 = p1 + p2
    p3 = p1 - p2
    p3 = p1 ** p2
    p3 = p1 /p2
    p3 = p1 % p2
    p3 = p1 >> p2
    p3 = p1 << p2
    p3 = p1 < p2
    p3 = p1 <= p2
    p3 = p1 > p2
    p3 = p1 >= p2
    p3 = p1 >= p2
    p3 = p1 >= p2
    p3 = p1 != p2
    p3 = p1 == p2
    p3 = p1 & p2
    p3 = p1 ^ p2
    p3 = p1 | p2
    p2 = -p1
    -p1
    p3 = p2 [ p1 ]

    p3 = p1 + 1
}
