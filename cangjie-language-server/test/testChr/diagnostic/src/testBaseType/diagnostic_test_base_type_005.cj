// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testBaseType
func testBaseType() {
    let a:int128 = 9
    var i32Number: int32 = 1024
    var f16Number: float16 = 0.0
    var f32Number: float32 = 0.0
    f16Number = float16(i32Number) // ok: f16Number = 1024.0
    f32Number = float32(i32Number) // ok: f32Number = 1024.0
    i32Number = 2147483647
    f16Number = float16(i32Number) // f16Number = POSITIVE_INFINITY
    f32Number = float32(i32Number) // precision lost: f32Number =
    2.14748365E9
    f32Number = 1024.1024
    i32Number = int32(f32Number) // ok: i32Number = 1024
    f32Number = 1024e10
    i32Number = int32(f32Number) // throw an ArithmeticException:(Int32)f32Number is not in the range of Int32
    f32Number = 3.4e40 // f32Number = POSITIVE_INFINITY
    i32Number = i32(f32Number) // throw an ArithmeticException
    f32Number = 3.4e40 * 0.0 // f32Number = NaN
    i32Number = int32(f32Number) // throw an ArithmeticException
    i32Number = (int32)f32Number
    return 0
}



