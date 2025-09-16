// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package currentDirectory
@C
struct P1{

}
foreign func viod():Unit
foreign func bool(a:Bool,b:CPointer<Bool>):Bool
foreign func int8(a:Int8,b:CPointer<Int8>):Int8
foreign func uint8(a:UInt8,b:CPointer<UInt8>):UInt8
foreign func int16(a:Int16,b:CPointer<Int16>):Int16
foreign func uint16(a:UInt16,b:CPointer<UInt16>):UInt16
foreign func int32(a:Int32,b:CPointer<Int32>):Int32
foreign func uint32(a:UInt32,b:CPointer<UInt32>):UInt32
foreign func int64(a:Int64,b:CPointer<Int64>):Int64
foreign func uint64(a:UInt64,b:CPointer<UInt64>):UInt64
foreign func int(a:IntNative,b:CPointer<IntNative>):IntNative
foreign func uint(a:UIntNative,b:CPointer<UIntNative>):UIntNative
foreign func float32(a:Float32,b:CPointer<Float32>):Float32
foreign func float64(a:Float64,b:CPointer<Float64>):Float64
foreign func float642(a:Float64,b:CPointer<CPointer<Float64>>):Float64
foreign func cstring(a:CString,b:CPointer<CString>):CString
foreign func p1(a:P1,b:CPointer<P1>):P1
foreign func p2(a:P1,b:CPointer<CPointer<P1>>):P1
foreign func error(a:Float16):Unit
