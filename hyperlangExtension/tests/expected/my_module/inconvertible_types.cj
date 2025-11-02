// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/***********METHOD***********/
/**
	 * @brief printToConsole(s: string & string): void
	 */
public func printToConsole(s: JSValue/* FIXME: `string & string` */): Unit {
    hmsGlobalApiCall < Unit >( "my_module_inconvertibleTypes", "printToConsole", { ctx =>[s] })
}

/**
	 * @brief printToConsole(s: object[]): void
	 */
public func printToConsole(s: Array<JSValue/* FIXME: `object` */>): Unit {
    hmsGlobalApiCall < Unit >( "my_module_inconvertibleTypes", "printToConsole", { ctx =>[toJSArray < JSValue/* FIXME: `object` */ >(ctx, s, { ctx: JSContext, val: JSValue/* FIXME: `object` */ => val })] })
}


