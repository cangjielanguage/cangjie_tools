// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testpkg

public let one : int32 = 1
public let two : int32 = 2

public class pkg {
    var aaa:int32 = 1
    protected var one:int32 = 1
    private var a:int32 = 0
    static var aa:int32 = 0
    init(a: int32) {
        this.a = a
    }
    public func add(a: int32, b: int32){
        return a + b + this.one
    }
    open func add2(a: int32, b: int32) {
        return a + b
    }
    static func add3(a: int32, b: int32) {
        return a + b
    }
}

public struct Test{
    let width: int32 = 2
    let length: int32 = 5
    init(a:int32, b:int32) {
        width = a
        length = b   
    }

    func area(): int32 {
        return width * length
    }
}

public enum TimeUnit {
    Year | Month | Day | Hour
}

public func testpkgFunc(): int32 {
    return 1
}

public interface iPkg {
    let aclass: int32 = 1
    func foo() : int32
    default func getNum():int32 {
        return 1
    }
    
    static func getSum():int32 {
        return 1
    }
}
