// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

foreign func add(a: Int32, b: Float32): Int32

foreign func legacy_func(): Int32

foreign func cfoo1(a: CPointer<Int32>): Unit

foreign func cfoo2(a: VArray<Int32, $3>): Unit

foreign func main_(argc: Int32, argv: VArray<CString, $0>): Int32

public type callback = CFunc<(arg1: Int32) -> Unit>

foreign func set_callback(cb: callback): Unit

foreign func add2(a: Int32, b: Int32, func_: CFunc<(arg1: Int32, arg2: Int32) -> Int32>): Int32

@C
public struct Point3D {
    public let x: Int64
    public let y: Int64
    public let z: Int64

    public init(x: Int64, y: Int64, z: Int64) {
        this.x = x
        this.y = y
        this.z = z
    }
}

foreign func addPoint(p1: Point3D, p2: Point3D): Point3D
