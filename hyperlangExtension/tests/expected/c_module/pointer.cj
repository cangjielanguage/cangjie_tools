package my_module

// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
public const num: Int32 = 42

// float* floatPtr = nullptr;
public type callback_func = CFunc<(arg1: Int32, arg2: Int32) -> Int32>

foreign func testPointer(a: Int32): CPointer<Unit>
