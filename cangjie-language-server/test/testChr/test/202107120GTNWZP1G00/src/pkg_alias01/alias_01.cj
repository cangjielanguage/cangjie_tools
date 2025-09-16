// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.pkg_alias01

var var01 = 1
public var var02 = 2


func func01() {
    return 3
}

public func func02() {
    return 3
}

public func func02(a:Int32) {
    return 3
}

public func func02(a:Int32,b:Int32) {
    return 3
}





class cls01 {
    var var01 = 1
}

public class cls02 {
    var var02 = 1
}

internal class cls03 {
    var var01 = 1
}





struct rcd01 {
    var var01 = 1
}

public struct rcd02 {
    var var02 = 1
}


enum en01 {
    DAY | MONTH
}

public enum en02 {
    DAY | MONTH
}


interface I01 {
	func f01(): Unit
}

public interface I02 {
	func f02(): Unit
}





