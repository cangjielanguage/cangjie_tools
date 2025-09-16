// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

 public open class Test12 {
	public init( a:Int32) {}
	public init(a:Int64) {}
	public init( a!:Bool) {}
	public init(c!:String = "cc") {}
	public Test12(a:Int64,b!:Int64) {a+b}
	public init(a:Int64,b!:Int32,c!:Bool=true){}
    public var abc = 0
}
func test42(){
    var x1 = Test12(1)
    var x2 = Test12(1, b: 2)
    var x3 = Test12(1, b: 2,c:true)
    var x4 = x1.abc
    Test12()
}

