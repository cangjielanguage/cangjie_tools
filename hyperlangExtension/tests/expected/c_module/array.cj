// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

foreign func test(arr: VArray<Int32, $3>): Unit

foreign func strcat1(dest: VArray<UInt8, $100>, src: VArray<UInt8, $50>, n: Int32): CString
