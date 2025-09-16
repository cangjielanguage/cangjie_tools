// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package logging.job
public class Job{
    public func getNameSpace():String{
        "defaute"
    }
    public func getName():String{
        "cangjie"
    }
}
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
