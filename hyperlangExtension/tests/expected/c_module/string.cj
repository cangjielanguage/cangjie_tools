// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

foreign func test(a: CString): Unit

/*FIXME: Non-constant global variable details need to be verified and rewritten by user.*/
/* char * str = "Hello World\n" */

foreign func testfunc1(): CString

foreign func testfunc2(a: CString, b: CString): CString

foreign func testfunc3(a: CString, b: CString, c: CString, d: CString, e: CString): Int32
