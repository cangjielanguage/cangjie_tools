// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package alias.pkgA

public open class A {}

public type AA = A

public struct C {}

public type CC = C

public type INT = Int64

@C
public struct C_1 {}

public type CC_1 = C_1

public let zz1122: CFunc<(CC_1) -> CC_1> = {a => a}
public let zz123: (CC, AA) = (CC(), AA())
public let zz124: (CC, Int32) = (CC(), 9)
public let zz125: (CC, INT) = (CC(), 9)
public let zz126: (A, Int32) = (A(), 9)
public const zz127: INT = 10

public func zzf1123(zz126: (A, Int32)) {
}

public func zzf1124(zz124: (CC, Int32)): (CC, Int32) {
    return zz124
}

public func zzf1125(zz124: (CC, INT)) {
}

public let zz122u4: (AA) = (AA())
public let zzrr112: Range<INT> = Range<INT>(0, 10, 1, true, true, true)
public let zza123: Array<INT> = [0, 0, 0, 0] // Array whose element type is Int64

public class CA123 {
    public let cc121: INT = 10
    public let cc122: CFunc<(CC_1) -> CC_1> = {a => a}
    public let cc123: (CC, AA) = (CC(), AA())
    public let cc124: (CC, Int32) = (CC(), 9)
    public let cc125: (CC, INT) = (CC(), 9)
    public let cc126: (A, Int32) = (A(), 9)

    public func ccfoo1(a!: AA): AA {
        a
    }
}

public func zzfoo1(a!: AA): AA {
    a
}

public func zzfoo2(a: AA): AA {
    a
}

public let zz1233u6: (AA) -> AA = zzfoo2
