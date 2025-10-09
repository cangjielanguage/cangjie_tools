// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
package default
extend String {
    func printSize() {
        //print(this.size())
    }
}
func name( )  {
	"123".printSize() // 3
}

class Foo {}

extend Foo {
    /**
    * this extended function
    */
    public func fExtend() {}
    /*
    *var修饰
    */
    mut prop extendedPrpo: Int64 {
        get() {
            return extendedPrpo
        }
        set(v) {
            extendedPrpo = v + 1
        }
    }
    static mut prop errorPrpo: Int64 {
    
        get() {
            return errorPrpo
        }
        set(v) {
            errorPrpo = v + 0.1
        }
    }
    static mut prop staticPrpo: Int64 {
        get() {
            return 0
        }
        set(v) {
            staticPrpo = v
        }
    }

}
func test1() {
let a = Foo()
    a.fExtend()     // ok
    a.extendedPrpo  //ok
    Foo.staticPrpo  //ok
    return 0
}

class FooGenerics<T> {}
extend<T> FooGenerics<T> {} // ok
extend<R> FooGenerics<R> {} // error
extend FooGenerics {} // error

interface I1 {
func f1(): Unit
}
interface I2 {
func f2(): Unit
}
interface I3 {
func f3(): Unit
}
class MultipleInheritance {}
extend MultipleInheritance <: I1 & I2 & I3 {
func f1(): Unit {}
func f2(): Unit {}
func f3(): Unit {}
}

interface I {
func f(): Unit
}
class Single  {}
extend Single  <: I {
    func f(): Unit {}
}
func testInterfaceExtend() {
    var i1:I1 = MultipleInheritance()
    i1.f1()

    var multipleInheritance = MultipleInheritance()
    multipleInheritance.f1()

    var single  = Single ()
    single.f()

    var i:I = Single ()
    i.f()
}

enum TimeUnit {
    Year|
    Month(Int64)|Day(Int64,Int64)
}

extend TimeUnit {
    /**
    * this extended function of enum
    */
    public func enumFuncExtend() {}
    static public func staticFuncExtend() {}
    /*
    *var修饰
    */
    mut prop extendedPrpo: Int64 {
        get() {
            return extendedPrpo
        }
        set(v) {
            extendedPrpo = v + 1
        }
    }
    static mut prop errorPrpo: Int64 {
    
        get() {
            return errorPrpo
        }
        //error
        set(v) {
            errorPrpo = v + 0.1
        }
    }
    static mut prop staticPrpo: Int64 {
        get() {
            return 0
        }
        set(v) {
            staticPrpo = v
        }
    }
}

func testEumExtend() {
    TimeUnit.Day(1, 2).extendedPrpo //OK
    TimeUnit.Day(1, 2).errorPrpo //error
    TimeUnit.Year.extendedPrpo //OK
    TimeUnit.Year.staticPrpo //error
    TimeUnit.staticPrpo //OK
    TimeUnit.enumFuncExtend() //error
    var day = TimeUnit.Day(1, 2) //ok
    day.enumFuncExtend() //ok
    day.staticFuncExtend() //error
    TimeUnit.staticFuncExtend() //ok
}
