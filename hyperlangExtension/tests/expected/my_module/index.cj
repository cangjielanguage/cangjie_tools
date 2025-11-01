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
	 * @brief compress(src: Uint8Array | number[], maxSize?: number): Uint8Array
	 */
public func compress(src: Array<UInt8>, maxSize!: Option < Float64 >= None): Array<UInt8> {
    hmsGlobalApiCall < Array<UInt8> >( "my_module", "compress", { ctx =>[toJSArray < UInt8 >(ctx, src, { ctx: JSContext, val: UInt8 => val.toJSValue(ctx) }), maxSize.toJSValue(ctx)] })
}

/**
	 * @brief compress(src: Uint8Array | number[], maxSize?: number): Uint8Array
	 */
public func compress(src: Array<Float64>, maxSize!: Option < Float64 >= None): Array<UInt8> {
    hmsGlobalApiCall < Array<UInt8> >( "my_module", "compress", { ctx =>[toJSArray < Float64 >(ctx, src), maxSize.toJSValue(ctx)] })
}


