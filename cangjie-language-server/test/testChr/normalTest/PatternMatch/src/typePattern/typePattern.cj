// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.typePattern
open class Point {
    var x: Int32 = 1
    var y: Int32 = 2
    init(x: Int32, y: Int32) {
    this.x = x
    this.y = y
    }
}

open class Surface {
    var count: Int32 = 1
    var count1: Int32 = 2
    init(x: Int32, y: Int32) {
    this.count = x
    this.count1 = y
    }
}

class ColoredPoint <: Point {
var color: String = "green"
init(x: Int32, y: Int32, color: String) {
super(x, y)
this.color = color
}
}
let normalPt = Point(5,10)
let colorPt = ColoredPoint(8,24,"red")
let surface = Surface(5,10)
var rectangleArea1: Int32 = match (normalPt) {
case _: Point => normalPt.x * normalPt.y // matched
case _ => 0
}
var rectangleArea2: Int32 = match (colorPt) {
    case cpt: Point => cpt.y * cpt.x
    case cpt12: ColoredPoint => cpt12.x * cpt12.y //matched
    case _ => 0
}

var a = 100

var s = a+a+a+a
