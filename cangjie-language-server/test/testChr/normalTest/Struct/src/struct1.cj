// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.




struct Data {
    var width: Int64
    let length: Int64
    init(w: Int64, l: Int64) {
        width = w
        length = l
        let locVar:Int64 = 2
        locVar = 3 // error: cannot assign to value : 'locvar' is an initialized 'let' constant
    }
}


struct Rectangle {
    public var length : Int32           // 未初始化的非静态成员变量定义
    private var width : Int32 = 0       // 初始化的非静态成员变量定义
    var diagonalLength: Int32 = 0
    static var name : String = "Rec"    // 静态成员变量
    static var perimeter: Int32 = 2 * (this.length + this.width) // ERROR: 静态成员变量的初值表达式中不能调用非静态成员。
    static var area: Int32              // ERROR: 静态成员变量定义未提供初始值

    init() {}                       // ERROR: 成员变量 `length` 未被初始化

    init(length: Int32) {           // OK: 成员变量 `length` 完成初始化
        this.length = length            
    }
}

func checkRectangle() {
    var rect: Rectangle = Rectangle(3)
    print("${rect.length}")         // OK: 访问控制符为 public，可以在struct外访问
    print("${rect.width}")          // ERROR: 访问控制符为 private，不可以在struct外访问
    print("${rect.diagonalLength}") // OK: 访问控制符缺省时为 public，可以在struct外访问
    print(Rectangle.name)      // OK: 静态变量通过 struct 名字访问
    print(rect.name)           // ERROR: 静态变量不能通过 struct实例访问，要通过struct名字访问
    print("${Rectangle.diagonalLength}")    // ERROR: 非静态变量不能通过struct名字访问，要通过struct实例访问
}
