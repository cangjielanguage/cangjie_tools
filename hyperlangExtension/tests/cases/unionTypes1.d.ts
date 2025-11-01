// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// type alias
type a1 = string|number

// global func
export function test(a:a1) : string|number

export class A {
  a:a1
  // property
  b: boolean | string

  // member function
  test1(a:a1) : string|number

  test2(a: string|number): number
}
