// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package code.callHierarchy1
let f1 = {a: Int64, b: Int64 => (a+b)}

public open class A0{}

public open class A1{
    public init() {
        
    }
}

public open class A2{
    public A2(){}
    public operator func +(right:A2):A2{
        A2()
    }
    
}
public open class human{
    public open func lunch(): Bool{
        true
    }
}
public class People <:human{
    public func eat(food: String, num: Int64): Unit {
        println("今天吃了${num}份${food}")
    }
    public People(var a: Int64, var b: Int64) {
        this.a = a
        this.b = b
    }
    public func lunch(): Bool {
        let fruitPriceHandler: (name:String, price: Int64) -> Unit
        fruitPriceHandler = {n, p => println("fruit: ${n} price: ${p} yuan")}
        fruitPriceHandler("banana", 10)
        eat("面条", 4)
        true
    }

    public func day(): Unit {
        lunch()
    }
}

//本包内
func test11():Unit{
let sum : (Int64,Int64)->Int64 = {a,b =>a+b}/*本包内*/
sum(1,2)//本包内
let a00 = A0()//本包内
let a01 = A0()//本包内
let a10 = A1()//本包内
let a11 = A1()//本包内
let a20 = A2()//本包内
let a21 = A2()//本包内
let a22 = a20+a21//本包内
let human1 = human()//本包内
let People1 = People(1,2)//本包内
println("今天是周五")//本包内
}
