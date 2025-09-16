// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class newPoint{
    var x:Int32=1
    var y:Int32=2
    init (x:Int32,y:Int32){
    this.x=x
    this.y=y
    }
}
class ColoredPoint <: newPoint{
    var color: String='green'

    init(x:Int32,y:Int32,color:String){
        super(x,y)
        this.color=color
    }
}

main(): Int64 {
    let normalPt=newPoint(5,10)
    let colorPt=ColoredPoint(8,9,'red')
    var rectangleArea1:Int32=match(normalPt){
    case _: newPoint => normalPt.x * normalPt.y
    }
    var rectangleArea2: Int32=match(colorPt){
    case cpt:newPoint =>cpt.x*cpt.y
    }
    return 0
}
