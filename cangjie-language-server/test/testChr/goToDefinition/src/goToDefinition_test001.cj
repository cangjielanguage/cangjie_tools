// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package test.C

var atest : int32 = 1
let globalVar = 333

public enum Timeunit {
    Year | Month | Day | Hour
}

public class Basec {
    var atest = 1
    protected var two = 2
    private var seven = 7
    static var three = 3
    protected static var five = 5
    private static var six = 6
    
    var par1 = atest
    init() {
        var par2 = atest
    }

    func test() {
        var par3 = atest
        var four = three
        var time1 = Timeunit.Year
        var time2 = .Month
        time1 = .Day
        time2 = Timeunit.Hour
        return four
    }
    class bb {
        var bb = 1
        func numberb(){

        }
    }
    static func test1() {
        var b1 = cc()
        
    }
    interface testinterface {
        let o = 11
        
    }
    var aaaaa = test()
}

class cc {
    var cc = 3
    static var stacc = 4
    func numberc(){
        var i = Basec.three
    }
    static func numberstac(p1:int16,p2:int32,p3:uint16){
        
    }
}

public struct structer1 {
    var a : int32 = 0
    // var par8 : int32 = atest
    init(input:int32){
        a = input
        // var par9 : int32 = atest
    }
}

interface Basei {
    default func getNum() {
        return 1
    }
}

interface Basei1 { 
    let bas: int32 = 5
    default func getNum1() {
        return 3
    }
    interface Basei2 {
        let m = 22
    }
}

open class Data <: Basec & Basei & Basei1 {
    var resc = test()
    var resi = getNum()
    var resi1 = two
    var par4 = atest
    init(){
        var test = 2
        var par5 = atest
    }
}

class AAA <: Data{
    var par6 = atest
    init() {
        var par7 = atest
    }
}



func run(): int64 {
    var value : Data = Data()
    var par10 = atest
    var test : structer1 = structer1(4)
    var i = test.a
    var e = atest
    if ( value.resc != 1 || value.resi != 1 || value.resi1 != 2 ) {
        return 1
    }
    return 0
}

func main(): int64 {
    var par11 = atest
    return run()
}

func test55(){
    var value1 : Data = Data()
}

class cc22 {
    var par12 = atest
    var testa = 2
    var testb = Basec()
    func aaa(){
        var par13 = atest
        var ccc = Basec()
    }
    init() {
        var par14 = atest
    }
}
